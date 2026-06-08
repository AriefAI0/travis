#include "mediaEngine/session/media_source_session.h"

#include "mediaEngine/pipeline/errors.h"

#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <iostream>
#include <mutex>

// Manages reusable live-source pipelines so multiple engine consumers can share one source.

namespace travis::media_engine::session {

namespace {

std::atomic<int> sessionCounter{0};

constexpr auto kSourceStartTimeout = std::chrono::seconds(30);
constexpr auto kSessionGracePeriod = std::chrono::seconds(2);
constexpr auto kBranchReleaseTimeout = std::chrono::seconds(2);
constexpr const char* kRecordingBridgeChannelPrefix = "travis-recording:";

struct BranchReleaseContext {
    MediaSourceSession* session = nullptr;
    GstPad* teeSrcPad = nullptr;
    GstElement* queue = nullptr;
    bool done = false;
    bool ok = false;
    std::string message;
    std::mutex mutex;
    std::condition_variable condition;
};

std::string trimCopy(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });

    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();

    if (first >= last) {
        return "";
    }

    return std::string(first, last);
}

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

GstPadProbeReturn onSessionSourceBuffer(GstPad*, GstPadProbeInfo* info, gpointer userData) {
    auto* session = static_cast<MediaSourceSession*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) == 0) {
        return GST_PAD_PROBE_OK;
    }

    session->receivedVideoBuffer = true;
    GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    std::cerr << "[RecordingDebug] Media source session first buffer"
              << " sessionKey=" << session->identity.key
              << " sourceKind=" << session->identity.kind
              << " sourceName=" << session->identity.sourceName
              << " pts=" << (buffer != nullptr ? GST_BUFFER_PTS(buffer) : GST_CLOCK_TIME_NONE)
              << " dts=" << (buffer != nullptr ? GST_BUFFER_DTS(buffer) : GST_CLOCK_TIME_NONE)
              << " duration="
              << (buffer != nullptr ? GST_BUFFER_DURATION(buffer) : GST_CLOCK_TIME_NONE)
              << " offset="
              << (buffer != nullptr ? GST_BUFFER_OFFSET(buffer) : GST_BUFFER_OFFSET_NONE)
              << " delta="
              << (buffer != nullptr &&
                  GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_DELTA_UNIT))
              << " discont="
              << (buffer != nullptr &&
                  GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_DISCONT))
              << std::endl;
    return GST_PAD_PROBE_REMOVE;
}

GstPadProbeReturn onBranchReleaseBlocked(GstPad*, GstPadProbeInfo*, gpointer userData) {
    auto* context = static_cast<BranchReleaseContext*>(userData);
    bool ok = true;
    std::string message = "Media source branch released";

    GstPad* queueSinkPad = nullptr;
    if (context->queue != nullptr) {
        queueSinkPad = gst_element_get_static_pad(context->queue, "sink");
    }

    if (queueSinkPad != nullptr) {
        if (gst_pad_is_linked(context->teeSrcPad)) {
            gst_pad_unlink(context->teeSrcPad, queueSinkPad);
        }
        gst_object_unref(queueSinkPad);
    } else {
        ok = false;
        message = "Failed to inspect media source branch queue sink pad during release";
    }

    gst_element_release_request_pad(context->session->tee, context->teeSrcPad);

    {
        std::lock_guard<std::mutex> lock(context->mutex);
        context->ok = ok;
        context->message = std::move(message);
        context->done = true;
    }
    context->condition.notify_one();

    return GST_PAD_PROBE_REMOVE;
}

} // namespace

MediaSourceSessionManager::~MediaSourceSessionManager() {
    stopAll();
}

MediaSourceIdentity createNdiSourceIdentity(const std::string& sourceName) {
    const auto normalizedSourceName = trimCopy(sourceName);

    return MediaSourceIdentity{
        .kind = "ndi",
        .sourceName = normalizedSourceName,
        .key = "ndi:" + lowerCopy(normalizedSourceName),
    };
}

MediaSourceIdentity createDeviceCaptureSourceIdentity(
    const std::string& deviceName,
    const std::string& devicePath,
    const std::string& sourceElement
) {
    const auto normalizedDeviceName = trimCopy(deviceName);
    const auto normalizedDevicePath = trimCopy(devicePath);
    const auto normalizedSourceElement =
        trimCopy(sourceElement).empty() ? "mfvideosrc" : trimCopy(sourceElement);
    const auto sourceReference =
        !normalizedDevicePath.empty() ? normalizedDevicePath : normalizedDeviceName;

    return MediaSourceIdentity{
        .kind = "device-capture",
        .sourceName = normalizedDeviceName,
        .key = "device-capture:" + lowerCopy(normalizedSourceElement + ":" + sourceReference),
    };
}

MediaSourceSessionResult MediaSourceSessionManager::acquireNdiSession(
    const std::string& sourceName,
    const std::string& urlAddress,
    MediaSourceSession*& session
) {
    const auto identity = createNdiSourceIdentity(sourceName);

    if (identity.sourceName.empty()) {
        session = nullptr;
        return MediaSourceSessionResult{false, "NDI sourceName is required"};
    }

    cleanupExpiredSessions();

    const auto existingSession = sessions_.find(identity.key);
    if (existingSession != sessions_.end()) {
        existingSession->second->pendingRemovalTime = std::nullopt;
        existingSession->second->consumerCount += 1;
        session = existingSession->second.get();
        return MediaSourceSessionResult{true, "NDI media source session acquired"};
    }

    std::unique_ptr<MediaSourceSession> createdSession;
    const auto createResult = createNdiSession(identity.sourceName, urlAddress, createdSession);

    if (!createResult.ok) {
        session = nullptr;
        return createResult;
    }

    createdSession->consumerCount = 1;
    session = createdSession.get();
    sessions_[createdSession->identity.key] = std::move(createdSession);
    return MediaSourceSessionResult{true, "NDI media source session started"};
}

MediaSourceSessionResult MediaSourceSessionManager::acquireDeviceCaptureSession(
    const std::string& deviceName,
    const std::string& devicePath,
    const std::string& sourceElement,
    MediaSourceSession*& session
) {
    const auto identity = createDeviceCaptureSourceIdentity(deviceName, devicePath, sourceElement);

    if (identity.sourceName.empty()) {
        session = nullptr;
        return MediaSourceSessionResult{false, "Device capture name is required"};
    }

    cleanupExpiredSessions();

    const auto existingSession = sessions_.find(identity.key);
    if (existingSession != sessions_.end()) {
        existingSession->second->pendingRemovalTime = std::nullopt;
        existingSession->second->consumerCount += 1;
        session = existingSession->second.get();
        return MediaSourceSessionResult{true, "Device capture media source session acquired"};
    }

    std::unique_ptr<MediaSourceSession> createdSession;
    const auto createResult = createDeviceCaptureSession(
        identity.sourceName,
        devicePath,
        sourceElement,
        createdSession
    );

    if (!createResult.ok) {
        session = nullptr;
        return createResult;
    }

    createdSession->consumerCount = 1;
    session = createdSession.get();
    sessions_[createdSession->identity.key] = std::move(createdSession);
    return MediaSourceSessionResult{true, "Device capture media source session started"};
}

MediaSourceSessionResult MediaSourceSessionManager::attachBranch(
    MediaSourceSession& session,
    GstElement* queue,
    AttachedSessionBranch& branch
) {
    if (queue == nullptr) {
        return MediaSourceSessionResult{false, "Branch queue is required"};
    }

    GstPad* teeSrcPad = gst_element_request_pad_simple(session.tee, "src_%u");
    GstPad* queueSinkPad = gst_element_get_static_pad(queue, "sink");

    if (teeSrcPad == nullptr || queueSinkPad == nullptr) {
        if (teeSrcPad != nullptr) {
            gst_object_unref(teeSrcPad);
        }
        if (queueSinkPad != nullptr) {
            gst_object_unref(queueSinkPad);
        }

        return MediaSourceSessionResult{false, "Failed to prepare media source branch pads"};
    }

    if (gst_pad_link(teeSrcPad, queueSinkPad) != GST_PAD_LINK_OK) {
        gst_object_unref(queueSinkPad);
        gst_element_release_request_pad(session.tee, teeSrcPad);
        gst_object_unref(teeSrcPad);
        return MediaSourceSessionResult{false, "Failed to attach branch to media source session"};
    }

    gst_object_unref(queueSinkPad);
    branch = AttachedSessionBranch{&session, queue, teeSrcPad};
    return MediaSourceSessionResult{true, "Media source branch attached"};
}

MediaSourceSessionResult MediaSourceSessionManager::releaseBranch(AttachedSessionBranch& branch) {
    if (branch.session == nullptr || branch.teeSrcPad == nullptr) {
        branch = AttachedSessionBranch{};
        return MediaSourceSessionResult{true, "Media source branch already released"};
    }

    BranchReleaseContext context{
        .session = branch.session,
        .teeSrcPad = branch.teeSrcPad,
        .queue = branch.queue,
    };

    const gulong probeId = gst_pad_add_probe(
        branch.teeSrcPad,
        GST_PAD_PROBE_TYPE_IDLE,
        onBranchReleaseBlocked,
        &context,
        nullptr
    );

    if (probeId == 0) {
        {
            std::lock_guard<std::mutex> lock(context.mutex);
            if (context.done) {
                const auto result = MediaSourceSessionResult{context.ok, context.message};
                gst_object_unref(branch.teeSrcPad);
                branch = AttachedSessionBranch{};
                return result;
            }
        }

        gst_element_release_request_pad(branch.session->tee, branch.teeSrcPad);
        gst_object_unref(branch.teeSrcPad);
        branch = AttachedSessionBranch{};
        return MediaSourceSessionResult{
            false,
            "Failed to block media source branch before release",
        };
    }

    bool completed = false;
    {
        std::unique_lock<std::mutex> lock(context.mutex);
        completed = context.condition.wait_for(
            lock,
            kBranchReleaseTimeout,
            [&context]() { return context.done; }
        );
    }

    if (!completed) {
        gst_pad_remove_probe(branch.teeSrcPad, probeId);
        gst_element_release_request_pad(branch.session->tee, branch.teeSrcPad);
        gst_object_unref(branch.teeSrcPad);
        branch = AttachedSessionBranch{};
        return MediaSourceSessionResult{
            false,
            "Timed out blocking media source branch before release",
        };
    }

    const auto result = MediaSourceSessionResult{context.ok, context.message};
    gst_object_unref(branch.teeSrcPad);
    branch = AttachedSessionBranch{};
    return result;
}

MediaSourceSessionResult MediaSourceSessionManager::ensureRecordingBridge(MediaSourceSession& session) {
    if (session.recordingBridgeSink != nullptr && !session.recordingBridgeChannel.empty()) {
        return MediaSourceSessionResult{true, "Recording bridge already available"};
    }

    session.recordingBridgeQueue = gst_element_factory_make("queue", nullptr);
    session.recordingBridgeSink = gst_element_factory_make("intervideosink", nullptr);
    session.recordingBridgeChannel = kRecordingBridgeChannelPrefix + session.identity.key;

    if (session.recordingBridgeQueue == nullptr || session.recordingBridgeSink == nullptr) {
        return MediaSourceSessionResult{false, "Failed to create media source recording bridge"};
    }

    g_object_set(session.recordingBridgeQueue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(
        session.recordingBridgeSink,
        "channel",
        session.recordingBridgeChannel.c_str(),
        "sync",
        FALSE,
        nullptr
    );

    gst_bin_add_many(
        GST_BIN(session.pipeline),
        session.recordingBridgeQueue,
        session.recordingBridgeSink,
        nullptr
    );

    if (!gst_element_link_many(
            session.recordingBridgeQueue,
            session.recordingBridgeSink,
            nullptr
        )) {
        gst_bin_remove_many(
            GST_BIN(session.pipeline),
            session.recordingBridgeQueue,
            session.recordingBridgeSink,
            nullptr
        );
        session.recordingBridgeQueue = nullptr;
        session.recordingBridgeSink = nullptr;
        session.recordingBridgeChannel.clear();
        return MediaSourceSessionResult{false, "Failed to link media source recording bridge"};
    }

    session.recordingBridgeTeeSrcPad = gst_element_request_pad_simple(session.tee, "src_%u");
    GstPad* bridgeQueueSinkPad = gst_element_get_static_pad(session.recordingBridgeQueue, "sink");

    if (session.recordingBridgeTeeSrcPad == nullptr || bridgeQueueSinkPad == nullptr) {
        if (bridgeQueueSinkPad != nullptr) {
            gst_object_unref(bridgeQueueSinkPad);
        }
        if (session.recordingBridgeTeeSrcPad != nullptr) {
            gst_element_release_request_pad(session.tee, session.recordingBridgeTeeSrcPad);
            gst_object_unref(session.recordingBridgeTeeSrcPad);
            session.recordingBridgeTeeSrcPad = nullptr;
        }
        gst_bin_remove_many(
            GST_BIN(session.pipeline),
            session.recordingBridgeQueue,
            session.recordingBridgeSink,
            nullptr
        );
        session.recordingBridgeQueue = nullptr;
        session.recordingBridgeSink = nullptr;
        session.recordingBridgeChannel.clear();
        return MediaSourceSessionResult{false, "Failed to prepare media source recording bridge pads"};
    }

    if (gst_pad_link(session.recordingBridgeTeeSrcPad, bridgeQueueSinkPad) != GST_PAD_LINK_OK) {
        gst_object_unref(bridgeQueueSinkPad);
        gst_element_release_request_pad(session.tee, session.recordingBridgeTeeSrcPad);
        gst_object_unref(session.recordingBridgeTeeSrcPad);
        session.recordingBridgeTeeSrcPad = nullptr;
        gst_bin_remove_many(
            GST_BIN(session.pipeline),
            session.recordingBridgeQueue,
            session.recordingBridgeSink,
            nullptr
        );
        session.recordingBridgeQueue = nullptr;
        session.recordingBridgeSink = nullptr;
        session.recordingBridgeChannel.clear();
        return MediaSourceSessionResult{false, "Failed to attach media source recording bridge"};
    }

    gst_object_unref(bridgeQueueSinkPad);
    gst_element_sync_state_with_parent(session.recordingBridgeSink);
    gst_element_sync_state_with_parent(session.recordingBridgeQueue);

    return MediaSourceSessionResult{true, "Media source recording bridge available"};
}

void MediaSourceSessionManager::releaseSession(MediaSourceSession& session) {
    if (session.consumerCount > 0) {
        session.consumerCount -= 1;
    }

    if (session.consumerCount == 0) {
        removeSession(session.identity.key);
    }
}

void MediaSourceSessionManager::stopAll() {
    while (!sessions_.empty()) {
        removeSession(sessions_.begin()->first);
    }
}

MediaSourceSessionResult MediaSourceSessionManager::createNdiSession(
    const std::string& sourceName,
    const std::string& urlAddress,
    std::unique_ptr<MediaSourceSession>& session
) {
    auto createdSession = std::unique_ptr<MediaSourceSession>(new MediaSourceSession{
        .identity = createNdiSourceIdentity(sourceName),
        .pipeline = gst_pipeline_new(("ndi-session-" + std::to_string(sessionCounter++)).c_str()),
        .ndiSource = travis::media_engine::sources::ndi::createNdiVideoSource(sourceName, urlAddress),
        .deviceCaptureSource = travis::media_engine::sources::device_capture::DeviceCaptureVideoSource{},
        .sourceQueue = gst_element_factory_make("queue", nullptr),
        .convert = gst_element_factory_make("videoconvert", nullptr),
        .tee = gst_element_factory_make("tee", nullptr),
        .warmupQueue = gst_element_factory_make("queue", nullptr),
        .warmupSink = gst_element_factory_make("fakesink", nullptr),
    });

    if (createdSession->pipeline == nullptr ||
        !travis::media_engine::sources::ndi::isNdiVideoSourceValid(createdSession->ndiSource) ||
        createdSession->sourceQueue == nullptr ||
        createdSession->convert == nullptr ||
        createdSession->tee == nullptr ||
        createdSession->warmupQueue == nullptr ||
        createdSession->warmupSink == nullptr) {
        if (createdSession->pipeline != nullptr) {
            gst_object_unref(createdSession->pipeline);
        }
        return MediaSourceSessionResult{false, "Failed to create NDI media source session elements"};
    }

    g_object_set(createdSession->sourceQueue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(createdSession->warmupQueue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(createdSession->warmupSink, "sync", FALSE, nullptr);

    travis::media_engine::sources::ndi::addNdiVideoSourceToBin(
        GST_BIN(createdSession->pipeline),
        createdSession->ndiSource
    );
    gst_bin_add_many(
        GST_BIN(createdSession->pipeline),
        createdSession->sourceQueue,
        createdSession->convert,
        createdSession->tee,
        createdSession->warmupQueue,
        createdSession->warmupSink,
        nullptr
    );

    if (!travis::media_engine::sources::ndi::linkNdiVideoSource(createdSession->ndiSource) ||
        !gst_element_link_many(
            createdSession->sourceQueue,
            createdSession->convert,
            createdSession->tee,
            nullptr
        ) ||
        !gst_element_link_many(
            createdSession->tee,
            createdSession->warmupQueue,
            createdSession->warmupSink,
            nullptr
        )) {
        gst_object_unref(createdSession->pipeline);
        return MediaSourceSessionResult{false, "Failed to link NDI media source session pipeline"};
    }

    travis::media_engine::sources::ndi::connectNdiVideoSourceToQueue(
        createdSession->ndiSource,
        createdSession->sourceQueue,
        createdSession->linkedVideoPad
    );

    GstPad* sourceQueueSrcPad = gst_element_get_static_pad(createdSession->sourceQueue, "src");
    if (sourceQueueSrcPad == nullptr) {
        gst_object_unref(createdSession->pipeline);
        return MediaSourceSessionResult{
            false,
            "Failed to inspect NDI media source queue src pad",
        };
    }

    gst_pad_add_probe(
        sourceQueueSrcPad,
        GST_PAD_PROBE_TYPE_BUFFER,
        onSessionSourceBuffer,
        createdSession.get(),
        nullptr
    );
    gst_object_unref(sourceQueueSrcPad);

    const GstStateChangeReturn stateResult =
        gst_element_set_state(createdSession->pipeline, GST_STATE_PLAYING);

    if (stateResult == GST_STATE_CHANGE_FAILURE) {
        GstBus* bus = gst_element_get_bus(createdSession->pipeline);
        GstMessage* message = gst_bus_timed_pop_filtered(bus, GST_SECOND, GST_MESSAGE_ERROR);

        std::string failureMessage = "Failed to start NDI media source session pipeline";
        if (message != nullptr) {
            failureMessage = pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
        }

        gst_object_unref(bus);
        gst_object_unref(createdSession->pipeline);
        return MediaSourceSessionResult{false, failureMessage};
    }

    const auto startResult = waitForFirstBuffer(*createdSession);
    if (!startResult.ok) {
        gst_element_set_state(createdSession->pipeline, GST_STATE_NULL);
        gst_object_unref(createdSession->pipeline);
        return startResult;
    }

    session = std::move(createdSession);
    return MediaSourceSessionResult{true, "NDI media source session started"};
}

MediaSourceSessionResult MediaSourceSessionManager::createDeviceCaptureSession(
    const std::string& deviceName,
    const std::string& devicePath,
    const std::string& sourceElement,
    std::unique_ptr<MediaSourceSession>& session
) {
    auto createdSession = std::unique_ptr<MediaSourceSession>(new MediaSourceSession{
        .identity = createDeviceCaptureSourceIdentity(deviceName, devicePath, sourceElement),
        .pipeline = gst_pipeline_new(
            ("device-capture-session-" + std::to_string(sessionCounter++)).c_str()
        ),
        .ndiSource = travis::media_engine::sources::ndi::NdiVideoSource{},
        .deviceCaptureSource =
            travis::media_engine::sources::device_capture::createDeviceCaptureVideoSource(
                deviceName,
                devicePath,
                sourceElement
            ),
        .sourceQueue = gst_element_factory_make("queue", nullptr),
        .convert = gst_element_factory_make("videoconvert", nullptr),
        .tee = gst_element_factory_make("tee", nullptr),
        .warmupQueue = gst_element_factory_make("queue", nullptr),
        .warmupSink = gst_element_factory_make("fakesink", nullptr),
    });

    if (createdSession->pipeline == nullptr ||
        !travis::media_engine::sources::device_capture::isDeviceCaptureVideoSourceValid(
            createdSession->deviceCaptureSource
        ) ||
        createdSession->sourceQueue == nullptr ||
        createdSession->convert == nullptr ||
        createdSession->tee == nullptr ||
        createdSession->warmupQueue == nullptr ||
        createdSession->warmupSink == nullptr) {
        if (createdSession->pipeline != nullptr) {
            gst_object_unref(createdSession->pipeline);
        }

        return MediaSourceSessionResult{
            false,
            "Failed to create device capture media source session elements",
        };
    }

    g_object_set(createdSession->sourceQueue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(createdSession->warmupQueue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(createdSession->warmupSink, "sync", FALSE, nullptr);

    travis::media_engine::sources::device_capture::addDeviceCaptureVideoSourceToBin(
        GST_BIN(createdSession->pipeline),
        createdSession->deviceCaptureSource
    );
    gst_bin_add_many(
        GST_BIN(createdSession->pipeline),
        createdSession->sourceQueue,
        createdSession->convert,
        createdSession->tee,
        createdSession->warmupQueue,
        createdSession->warmupSink,
        nullptr
    );

    if (!travis::media_engine::sources::device_capture::linkDeviceCaptureVideoSourceToQueue(
            createdSession->deviceCaptureSource,
            createdSession->sourceQueue
        ) ||
        !gst_element_link_many(
            createdSession->sourceQueue,
            createdSession->convert,
            createdSession->tee,
            nullptr
        ) ||
        !gst_element_link_many(
            createdSession->tee,
            createdSession->warmupQueue,
            createdSession->warmupSink,
            nullptr
        )) {
        gst_object_unref(createdSession->pipeline);
        return MediaSourceSessionResult{
            false,
            "Failed to link device capture media source session pipeline",
        };
    }

    GstPad* sourceQueueSrcPad = gst_element_get_static_pad(createdSession->sourceQueue, "src");
    if (sourceQueueSrcPad == nullptr) {
        gst_object_unref(createdSession->pipeline);
        return MediaSourceSessionResult{
            false,
            "Failed to inspect device capture media source queue src pad",
        };
    }

    gst_pad_add_probe(
        sourceQueueSrcPad,
        GST_PAD_PROBE_TYPE_BUFFER,
        onSessionSourceBuffer,
        createdSession.get(),
        nullptr
    );
    gst_object_unref(sourceQueueSrcPad);

    const GstStateChangeReturn stateResult =
        gst_element_set_state(createdSession->pipeline, GST_STATE_PLAYING);

    if (stateResult == GST_STATE_CHANGE_FAILURE) {
        GstBus* bus = gst_element_get_bus(createdSession->pipeline);
        GstMessage* message = gst_bus_timed_pop_filtered(bus, GST_SECOND, GST_MESSAGE_ERROR);

        std::string failureMessage = "Failed to start device capture media source session pipeline";
        if (message != nullptr) {
            failureMessage = pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
        }

        gst_object_unref(bus);
        gst_object_unref(createdSession->pipeline);
        return MediaSourceSessionResult{false, failureMessage};
    }

    const auto startResult = waitForFirstBuffer(*createdSession);
    if (!startResult.ok) {
        gst_element_set_state(createdSession->pipeline, GST_STATE_NULL);
        gst_object_unref(createdSession->pipeline);
        return startResult;
    }

    session = std::move(createdSession);
    return MediaSourceSessionResult{true, "Device capture media source session started"};
}

MediaSourceSessionResult MediaSourceSessionManager::waitForFirstBuffer(MediaSourceSession& session) {
    GstBus* bus = gst_element_get_bus(session.pipeline);
    const auto deadline = std::chrono::steady_clock::now() + kSourceStartTimeout;

    while (std::chrono::steady_clock::now() < deadline) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            bus,
            100 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
        );

        if (message != nullptr) {
            switch (GST_MESSAGE_TYPE(message)) {
            case GST_MESSAGE_ERROR: {
                const auto failureMessage = pipeline::readGstErrorMessage(message);
                gst_message_unref(message);
                gst_object_unref(bus);
                return MediaSourceSessionResult{false, failureMessage};
            }
            case GST_MESSAGE_EOS:
                gst_message_unref(message);
                gst_object_unref(bus);
                return MediaSourceSessionResult{
                    false,
                    "Media source ended before video was available",
                };
            default:
                gst_message_unref(message);
                break;
            }
        }

        if (session.receivedVideoBuffer) {
            gst_object_unref(bus);
            return MediaSourceSessionResult{true, "Media source session started"};
        }
    }

    gst_object_unref(bus);
    return MediaSourceSessionResult{
        false,
        "Timed out waiting for media source session video buffer",
    };
}

void MediaSourceSessionManager::cleanupExpiredSessions() {
    const auto now = std::chrono::steady_clock::now();
    auto it = sessions_.begin();

    while (it != sessions_.end()) {
        const auto& session = *it->second;
        if (session.pendingRemovalTime &&
            (now - *session.pendingRemovalTime) >= kSessionGracePeriod) {
            removeSession(it->first);
            it = sessions_.begin();
        } else {
            ++it;
        }
    }
}

void MediaSourceSessionManager::removeSession(const std::string& sessionKey) {
    const auto session = sessions_.find(sessionKey);
    if (session == sessions_.end()) {
        return;
    }

    gst_element_set_state(session->second->pipeline, GST_STATE_NULL);
    gst_element_get_state(session->second->pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);

    if (session->second->recordingBridgeTeeSrcPad != nullptr) {
        gst_element_release_request_pad(session->second->tee, session->second->recordingBridgeTeeSrcPad);
        gst_object_unref(session->second->recordingBridgeTeeSrcPad);
        session->second->recordingBridgeTeeSrcPad = nullptr;
    }

    gst_object_unref(session->second->pipeline);
    sessions_.erase(session);
}

} // namespace travis::media_engine::session
