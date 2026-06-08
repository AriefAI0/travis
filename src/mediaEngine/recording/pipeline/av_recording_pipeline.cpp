#include "mediaEngine/recording/pipeline/av_recording_pipeline.h"

#include "mediaEngine/pipeline/errors.h"
#include "mediaEngine/recording/shared/recording_validation.h"

#include <chrono>

// Builds the first dedicated in-process AV recording graph from shared live-source bridges.

namespace travis::media_engine::recording {

namespace {

constexpr auto kAvRecordingStartTimeout = std::chrono::seconds(30);
constexpr auto kAvRecordingStopTimeout = std::chrono::seconds(30);
constexpr gint64 kWasapiLatencyTime = 5000;
constexpr gint64 kWasapiBufferTime = 20000;
constexpr int kRecordingVideoWidth = 1920;
constexpr int kRecordingVideoHeight = 1080;

std::string resolveAudioSourceElement(const RecordingAudioInput& audioInput) {
    return audioInput.sourceElement.empty() ? "wasapi2src" : audioInput.sourceElement;
}

void setAudioSourceDevice(GstElement* source, const RecordingAudioInput& audioInput) {
    const std::string sourceElement = resolveAudioSourceElement(audioInput);

    if (!audioInput.devicePath.empty()) {
        g_object_set(source, "device", audioInput.devicePath.c_str(), nullptr);
    } else if (!audioInput.deviceName.empty()) {
        g_object_set(source, "device-name", audioInput.deviceName.c_str(), nullptr);
    }

    if (sourceElement == "wasapi2src") {
        g_object_set(
            source,
            "low-latency",
            TRUE,
            "latency-time",
            kWasapiLatencyTime,
            "buffer-time",
            kWasapiBufferTime,
            "do-timestamp",
            TRUE,
            nullptr
        );
    }
}

void setRecordingOutputValves(AvRecordingPipeline& pipeline, gboolean drop) {
    if (pipeline.videoOutputValve != nullptr) {
        g_object_set(pipeline.videoOutputValve, "drop", drop, nullptr);
    }

    if (pipeline.audioOutputValve != nullptr) {
        g_object_set(pipeline.audioOutputValve, "drop", drop, nullptr);
    }
}

GstPadProbeReturn onVideoBuffer(GstPad*, GstPadProbeInfo* info, gpointer userData) {
    auto* pipeline = static_cast<AvRecordingPipeline*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) != 0) {
        pipeline->receivedVideoBuffer = true;
        setRecordingOutputValves(*pipeline, FALSE);
    }

    return GST_PAD_PROBE_REMOVE;
}

GstPadProbeReturn onAudioBuffer(GstPad*, GstPadProbeInfo* info, gpointer userData) {
    auto* branch = static_cast<AvRecordingAudioInputBranch*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) != 0) {
        branch->receivedBuffer = true;
    }

    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn onMixedAudioBuffer(GstPad*, GstPadProbeInfo* info, gpointer userData) {
    auto* pipeline = static_cast<AvRecordingPipeline*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) != 0) {
        pipeline->receivedAudioBuffer = true;
    }

    return GST_PAD_PROBE_REMOVE;
}

RecordingResult waitForRecordingStart(AvRecordingPipeline& pipeline) {
    const auto deadline = std::chrono::steady_clock::now() + kAvRecordingStartTimeout;

    while (std::chrono::steady_clock::now() < deadline) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            pipeline.bus,
            100 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
        );

        if (message != nullptr) {
            switch (GST_MESSAGE_TYPE(message)) {
            case GST_MESSAGE_ERROR: {
                const auto failureMessage = travis::media_engine::pipeline::readGstErrorMessage(message);
                gst_message_unref(message);
                return RecordingResult{false, failureMessage};
            }
            case GST_MESSAGE_EOS:
                gst_message_unref(message);
                return RecordingResult{false, "Recording source ended before data was available"};
            default:
                gst_message_unref(message);
                break;
            }
        }

        bool audioReady = !pipeline.audioActive;
        if (pipeline.audioActive) {
            audioReady = pipeline.receivedAudioBuffer;
            for (const auto& branch : pipeline.audioInputBranches) {
                if (!branch.receivedBuffer) {
                    audioReady = false;
                    break;
                }
            }
        }

        if (pipeline.receivedVideoBuffer && audioReady) {
            return RecordingResult{true, "Recording started"};
        }
    }

    return RecordingResult{false, "Timed out waiting for recording data"};
}

RecordingResult prepareVideoPath(
    const std::string& recordingId,
    const std::vector<AvRecordingVideoInput>& videoInputs,
    AvRecordingPipeline& pipeline
) {
    if (videoInputs.size() != 1) {
        return RecordingResult{false, "Recording expects exactly one AV video input"};
    }

    const auto& videoInput = videoInputs.front();
    pipeline.videoSource = gst_element_factory_make("intervideosrc", nullptr);
    pipeline.videoQueue = gst_element_factory_make("queue", nullptr);
    pipeline.videoRate = gst_element_factory_make("videorate", nullptr);
    pipeline.videoConvert = gst_element_factory_make("videoconvert", nullptr);
    pipeline.videoScale = gst_element_factory_make("videoscale", nullptr);
    pipeline.videoCapsFilter = gst_element_factory_make("capsfilter", nullptr);
    pipeline.videoEncoder = gst_element_factory_make("qsvh264enc", nullptr);
    pipeline.videoParser = gst_element_factory_make("h264parse", nullptr);
    pipeline.videoH264CapsFilter = gst_element_factory_make("capsfilter", nullptr);
    pipeline.videoOutputValve = gst_element_factory_make("valve", nullptr);
    pipeline.muxer = gst_element_factory_make("matroskamux", nullptr);
    pipeline.sink = gst_element_factory_make("filesink", nullptr);

    if (pipeline.videoSource == nullptr ||
        pipeline.videoQueue == nullptr ||
        pipeline.videoRate == nullptr ||
        pipeline.videoConvert == nullptr ||
        pipeline.videoScale == nullptr ||
        pipeline.videoCapsFilter == nullptr ||
        pipeline.videoEncoder == nullptr ||
        pipeline.videoParser == nullptr ||
        pipeline.videoH264CapsFilter == nullptr ||
        pipeline.videoOutputValve == nullptr ||
        pipeline.muxer == nullptr ||
        pipeline.sink == nullptr) {
        return RecordingResult{false, "Failed to create recording video pipeline elements"};
    }

    g_object_set(
        pipeline.videoSource,
        "channel",
        videoInput.bridgeChannel.c_str(),
        "do-timestamp",
        TRUE,
        nullptr
    );
    g_object_set(pipeline.videoQueue, "max-size-buffers", 0, "max-size-bytes", 0, "max-size-time", 0, nullptr);
    g_object_set(pipeline.videoOutputValve, "drop", TRUE, "drop-mode", 1, nullptr);
    g_object_set(pipeline.videoEncoder, "target-usage", 4, nullptr);
    g_object_set(pipeline.sink, "location", pipeline.outputPath.c_str(), "sync", FALSE, nullptr);

    GstCaps* videoCaps = gst_caps_new_simple(
        "video/x-raw",
        "format",
        G_TYPE_STRING,
        "NV12",
        "width",
        G_TYPE_INT,
        videoInput.width > 0 ? videoInput.width : kRecordingVideoWidth,
        "height",
        G_TYPE_INT,
        videoInput.height > 0 ? videoInput.height : kRecordingVideoHeight,
        "framerate",
        GST_TYPE_FRACTION,
        30,
        1,
        nullptr
    );
    g_object_set(pipeline.videoCapsFilter, "caps", videoCaps, nullptr);
    gst_caps_unref(videoCaps);

    GstCaps* h264Caps = gst_caps_new_simple(
        "video/x-h264",
        "stream-format",
        G_TYPE_STRING,
        "avc",
        "alignment",
        G_TYPE_STRING,
        "au",
        nullptr
    );
    g_object_set(pipeline.videoH264CapsFilter, "caps", h264Caps, nullptr);
    gst_caps_unref(h264Caps);

    gst_bin_add_many(
        GST_BIN(pipeline.pipeline),
        pipeline.videoSource,
        pipeline.videoQueue,
        pipeline.videoRate,
        pipeline.videoConvert,
        pipeline.videoScale,
        pipeline.videoCapsFilter,
        pipeline.videoEncoder,
        pipeline.videoParser,
        pipeline.videoH264CapsFilter,
        pipeline.videoOutputValve,
        pipeline.muxer,
        pipeline.sink,
        nullptr
    );

    if (!gst_element_link_many(
            pipeline.videoSource,
            pipeline.videoQueue,
            pipeline.videoRate,
            pipeline.videoConvert,
            pipeline.videoScale,
            pipeline.videoCapsFilter,
            pipeline.videoEncoder,
            pipeline.videoParser,
            pipeline.videoH264CapsFilter,
            pipeline.videoOutputValve,
            pipeline.muxer,
            pipeline.sink,
            nullptr
        )) {
        return RecordingResult{false, "Failed to link recording video pipeline"};
    }

    GstPad* videoQueueSrcPad = gst_element_get_static_pad(pipeline.videoQueue, "src");
    if (videoQueueSrcPad == nullptr) {
        return RecordingResult{false, "Failed to inspect recording video queue src pad"};
    }

    gst_pad_add_probe(
        videoQueueSrcPad,
        GST_PAD_PROBE_TYPE_BUFFER,
        onVideoBuffer,
        &pipeline,
        nullptr
    );
    gst_object_unref(videoQueueSrcPad);

    (void)recordingId;
    return RecordingResult{true, "Recording video path prepared"};
}

RecordingResult prepareAudioPath(
    const std::vector<RecordingAudioInput>& audioInputs,
    AvRecordingPipeline& pipeline
) {
    if (audioInputs.empty()) {
        return RecordingResult{true, "Recording does not require audio inputs"};
    }

    pipeline.audioActive = true;
    pipeline.audioMixer = gst_element_factory_make("audiomixer", nullptr);
    pipeline.audioMixerQueue = gst_element_factory_make("queue", nullptr);
    pipeline.audioMixerConvert = gst_element_factory_make("audioconvert", nullptr);
    pipeline.audioMixerResample = gst_element_factory_make("audioresample", nullptr);
    pipeline.audioMixerCapsFilter = gst_element_factory_make("capsfilter", nullptr);
    pipeline.audioEncoder = gst_element_factory_make("opusenc", nullptr);
    pipeline.audioOutputValve = gst_element_factory_make("valve", nullptr);

    if (pipeline.audioMixer == nullptr ||
        pipeline.audioMixerQueue == nullptr ||
        pipeline.audioMixerConvert == nullptr ||
        pipeline.audioMixerResample == nullptr ||
        pipeline.audioMixerCapsFilter == nullptr ||
        pipeline.audioEncoder == nullptr ||
        pipeline.audioOutputValve == nullptr) {
        return RecordingResult{false, "Failed to create recording audio pipeline elements"};
    }

    g_object_set(
        pipeline.audioMixerQueue,
        "max-size-buffers",
        0,
        "max-size-bytes",
        0,
        "max-size-time",
        0,
        nullptr
    );
    g_object_set(pipeline.audioEncoder, "audio-type", 2049, nullptr);
    g_object_set(pipeline.audioOutputValve, "drop", TRUE, "drop-mode", 1, nullptr);

    GstCaps* mixedAudioCaps = gst_caps_new_simple(
        "audio/x-raw",
        "format",
        G_TYPE_STRING,
        "S16LE",
        "rate",
        G_TYPE_INT,
        48000,
        "channels",
        G_TYPE_INT,
        2,
        nullptr
    );
    g_object_set(pipeline.audioMixerCapsFilter, "caps", mixedAudioCaps, nullptr);
    gst_caps_unref(mixedAudioCaps);

    gst_bin_add_many(
        GST_BIN(pipeline.pipeline),
        pipeline.audioMixer,
        pipeline.audioMixerQueue,
        pipeline.audioMixerConvert,
        pipeline.audioMixerResample,
        pipeline.audioMixerCapsFilter,
        pipeline.audioEncoder,
        pipeline.audioOutputValve,
        nullptr
    );

    if (!gst_element_link_many(
            pipeline.audioMixer,
            pipeline.audioMixerQueue,
            pipeline.audioMixerConvert,
            pipeline.audioMixerResample,
            pipeline.audioMixerCapsFilter,
            pipeline.audioEncoder,
            pipeline.audioOutputValve,
            pipeline.muxer,
            nullptr
        )) {
        return RecordingResult{false, "Failed to link recording audio pipeline"};
    }

    GstPad* audioMixerQueueSrcPad = gst_element_get_static_pad(pipeline.audioMixerQueue, "src");
    if (audioMixerQueueSrcPad == nullptr) {
        return RecordingResult{false, "Failed to inspect recording audio mixer queue src pad"};
    }

    gst_pad_add_probe(
        audioMixerQueueSrcPad,
        GST_PAD_PROBE_TYPE_BUFFER,
        onMixedAudioBuffer,
        &pipeline,
        nullptr
    );
    gst_object_unref(audioMixerQueueSrcPad);

    pipeline.audioInputBranches.reserve(audioInputs.size());

    for (const auto& audioInput : audioInputs) {
        AvRecordingAudioInputBranch branch{
            .source = gst_element_factory_make(resolveAudioSourceElement(audioInput).c_str(), nullptr),
            .queue = gst_element_factory_make("queue", nullptr),
            .convert = gst_element_factory_make("audioconvert", nullptr),
            .resample = gst_element_factory_make("audioresample", nullptr),
            .capsFilter = gst_element_factory_make("capsfilter", nullptr),
        };

        if (branch.source == nullptr ||
            branch.queue == nullptr ||
            branch.convert == nullptr ||
            branch.resample == nullptr ||
            branch.capsFilter == nullptr) {
            return RecordingResult{false, "Failed to create recording audio input branch"};
        }

        setAudioSourceDevice(branch.source, audioInput);
        g_object_set(branch.queue, "max-size-buffers", 0, "max-size-bytes", 0, "max-size-time", 0, nullptr);

        GstCaps* audioCaps = gst_caps_new_simple(
            "audio/x-raw",
            "format",
            G_TYPE_STRING,
            "S16LE",
            "rate",
            G_TYPE_INT,
            48000,
            "channels",
            G_TYPE_INT,
            2,
            nullptr
        );
        g_object_set(branch.capsFilter, "caps", audioCaps, nullptr);
        gst_caps_unref(audioCaps);

        gst_bin_add_many(
            GST_BIN(pipeline.pipeline),
            branch.source,
            branch.queue,
            branch.convert,
            branch.resample,
            branch.capsFilter,
            nullptr
        );

        if (!gst_element_link_many(
                branch.source,
                branch.queue,
                branch.convert,
                branch.resample,
                branch.capsFilter,
                nullptr
            )) {
            return RecordingResult{false, "Failed to link recording audio input branch"};
        }

        GstPad* audioCapsSrcPad = gst_element_get_static_pad(branch.capsFilter, "src");
        branch.mixerSinkPad = gst_element_request_pad_simple(pipeline.audioMixer, "sink_%u");

        if (audioCapsSrcPad == nullptr || branch.mixerSinkPad == nullptr) {
            if (audioCapsSrcPad != nullptr) {
                gst_object_unref(audioCapsSrcPad);
            }
            return RecordingResult{false, "Failed to prepare recording audio mixer pads"};
        }

        if (gst_pad_link(audioCapsSrcPad, branch.mixerSinkPad) != GST_PAD_LINK_OK) {
            gst_object_unref(audioCapsSrcPad);
            return RecordingResult{false, "Failed to connect recording audio input to mixer"};
        }
        gst_object_unref(audioCapsSrcPad);

        GstPad* audioQueueSrcPad = gst_element_get_static_pad(branch.queue, "src");
        if (audioQueueSrcPad == nullptr) {
            return RecordingResult{false, "Failed to inspect recording audio queue src pad"};
        }

        pipeline.audioInputBranches.push_back(branch);
        gst_pad_add_probe(
            audioQueueSrcPad,
            GST_PAD_PROBE_TYPE_BUFFER,
            onAudioBuffer,
            &pipeline.audioInputBranches.back(),
            nullptr
        );
        gst_object_unref(audioQueueSrcPad);
    }

    return RecordingResult{true, "Recording audio path prepared"};
}

} // namespace

RecordingResult startAvRecordingPipeline(
    const std::string& recordingId,
    const std::vector<AvRecordingVideoInput>& videoInputs,
    const std::vector<RecordingAudioInput>& audioInputs,
    const std::string& outputPath,
    travis::media_engine::session::MediaSourceSessionManager* sessionManager,
    const std::vector<travis::media_engine::session::MediaSourceSession*>& sourceSessions,
    AvRecordingPipeline& pipeline
) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    if (outputPath.empty()) {
        return RecordingResult{false, "outputPath is required"};
    }

    pipeline = AvRecordingPipeline{};
    pipeline.recordingId = recordingId;
    pipeline.outputPath = outputPath;
    pipeline.sessionManager = sessionManager;
    pipeline.sourceSessions = sourceSessions;
    pipeline.pipeline = gst_pipeline_new(("recording-" + recordingId).c_str());

    if (pipeline.pipeline == nullptr) {
        return RecordingResult{false, "Failed to create recording pipeline"};
    }

    const auto videoPrepareResult = prepareVideoPath(recordingId, videoInputs, pipeline);
    if (!videoPrepareResult.ok) {
        removeAvRecordingPipeline(pipeline);
        return videoPrepareResult;
    }

    const auto audioPrepareResult = prepareAudioPath(audioInputs, pipeline);
    if (!audioPrepareResult.ok) {
        removeAvRecordingPipeline(pipeline);
        return audioPrepareResult;
    }

    pipeline.bus = gst_element_get_bus(pipeline.pipeline);

    const GstStateChangeReturn stateResult =
        gst_element_set_state(pipeline.pipeline, GST_STATE_PLAYING);
    if (stateResult == GST_STATE_CHANGE_FAILURE) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            pipeline.bus,
            GST_SECOND,
            GST_MESSAGE_ERROR
        );

        std::string failureMessage = "Failed to start recording pipeline";
        if (message != nullptr) {
            failureMessage = travis::media_engine::pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
        }

        removeAvRecordingPipeline(pipeline);
        return RecordingResult{false, failureMessage};
    }

    const auto startResult = waitForRecordingStart(pipeline);
    if (!startResult.ok) {
        removeAvRecordingPipeline(pipeline);
        return startResult;
    }

    pipeline.startedAt = std::chrono::steady_clock::now();
    return RecordingResult{true, "Recording started"};
}

RecordingResult stopAvRecordingPipeline(AvRecordingPipeline& pipeline) {
    if (pipeline.pipeline == nullptr || pipeline.bus == nullptr) {
        return RecordingResult{false, "Recording pipeline is not running"};
    }

    setRecordingOutputValves(pipeline, FALSE);

    if (!gst_element_send_event(pipeline.pipeline, gst_event_new_eos())) {
        removeAvRecordingPipeline(pipeline);
        return RecordingResult{false, "Failed to send EOS to recording pipeline"};
    }

    const auto deadline = std::chrono::steady_clock::now() + kAvRecordingStopTimeout;
    while (std::chrono::steady_clock::now() < deadline) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            pipeline.bus,
            100 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
        );

        if (message == nullptr) {
            continue;
        }

        switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            const auto failureMessage = travis::media_engine::pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
            removeAvRecordingPipeline(pipeline);
            return RecordingResult{false, failureMessage};
        }
        case GST_MESSAGE_EOS:
            gst_message_unref(message);
            removeAvRecordingPipeline(pipeline);
            return RecordingResult{true, "Recording stopped"};
        default:
            gst_message_unref(message);
            break;
        }
    }

    removeAvRecordingPipeline(pipeline);
    return RecordingResult{false, "Timed out waiting for recording finalization"};
}

void removeAvRecordingPipeline(AvRecordingPipeline& pipeline) {
    if (pipeline.audioMixer != nullptr) {
        for (auto& branch : pipeline.audioInputBranches) {
            if (branch.mixerSinkPad != nullptr) {
                gst_element_release_request_pad(pipeline.audioMixer, branch.mixerSinkPad);
                gst_object_unref(branch.mixerSinkPad);
                branch.mixerSinkPad = nullptr;
            }
        }
    }

    if (pipeline.pipeline != nullptr) {
        gst_element_set_state(pipeline.pipeline, GST_STATE_NULL);
        gst_element_get_state(pipeline.pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
    }

    if (pipeline.bus != nullptr) {
        gst_object_unref(pipeline.bus);
        pipeline.bus = nullptr;
    }

    if (pipeline.pipeline != nullptr) {
        gst_object_unref(pipeline.pipeline);
        pipeline.pipeline = nullptr;
    }

    if (pipeline.sessionManager != nullptr) {
        for (auto* sourceSession : pipeline.sourceSessions) {
            if (sourceSession != nullptr) {
                pipeline.sessionManager->releaseSession(*sourceSession);
            }
        }
    }

    pipeline = AvRecordingPipeline{};
}

} // namespace travis::media_engine::recording
