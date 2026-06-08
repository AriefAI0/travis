#include "mediaEngine/recording/pipeline/av_recording_pipeline.h"

#include "mediaEngine/pipeline/errors.h"
#include "mediaEngine/recording/shared/recording_finalization.h"
#include "mediaEngine/recording/shared/recording_validation.h"

#include <chrono>
#include <mutex>

#include <gst/video/video-event.h>

// Builds the in-process AV recording graph and optional clip branches from shared live-source bridges.

namespace travis::media_engine::recording {

namespace {

constexpr auto kAvRecordingStartTimeout = std::chrono::seconds(30);
constexpr auto kAvRecordingStopTimeout = std::chrono::seconds(30);
constexpr gint64 kWasapiLatencyTime = 5000;
constexpr gint64 kWasapiBufferTime = 20000;
constexpr int kRecordingVideoWidth = 1920;
constexpr int kRecordingVideoHeight = 1080;
constexpr const char* kPreferredRecordingVideoEncoders[] = {
    "qsvh264enc",
    "mfh264enc",
    "x264enc",
};

GstElement* createRecordingVideoEncoder(std::string& encoderName) {
    for (const char* candidate : kPreferredRecordingVideoEncoders) {
        if (GstElement* encoder = gst_element_factory_make(candidate, nullptr); encoder != nullptr) {
            encoderName = candidate;
            return encoder;
        }
    }

    encoderName.clear();
    return nullptr;
}

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

GstPadProbeReturn onClipBuffer(GstPad*, GstPadProbeInfo* info, gpointer userData) {
    auto* clipBranch = static_cast<AvInspectionClipBranch*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) == 0) {
        return GST_PAD_PROBE_OK;
    }

    GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (buffer == nullptr || GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_DELTA_UNIT)) {
        return GST_PAD_PROBE_OK;
    }

    if (clipBranch->valve != nullptr) {
        g_object_set(clipBranch->valve, "drop", FALSE, nullptr);
    }

    if (clipBranch->audioActive && clipBranch->audioValve != nullptr) {
        g_object_set(clipBranch->audioValve, "drop", FALSE, nullptr);
    }

    clipBranch->receivedBuffer = true;
    clipBranch->openedOnKeyframe = true;
    return GST_PAD_PROBE_REMOVE;
}

RecordingResult requestRecordingKeyframe(AvRecordingPipeline& pipeline) {
    if (pipeline.videoEncoder == nullptr) {
        return RecordingResult{false, "Recording video encoder is not available"};
    }

    GstPad* encoderSrcPad = gst_element_get_static_pad(pipeline.videoEncoder, "src");
    if (encoderSrcPad == nullptr) {
        return RecordingResult{false, "Failed to get recording encoder source pad"};
    }

    GstEvent* forceKeyUnit =
        gst_video_event_new_upstream_force_key_unit(GST_CLOCK_TIME_NONE, TRUE, 0);
    const gboolean keyframeRequested = gst_pad_send_event(encoderSrcPad, forceKeyUnit);
    gst_object_unref(encoderSrcPad);

    if (!keyframeRequested) {
        return RecordingResult{false, "Failed to request recording keyframe"};
    }

    return RecordingResult{true, "Recording keyframe requested"};
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

RecordingResult removeAvInspectionClipBranch(
    AvRecordingPipeline& pipeline,
    AvInspectionClipBranch& clipBranch
) {
    if (clipBranch.audioQueue != nullptr) {
        gst_element_set_state(clipBranch.audioQueue, GST_STATE_NULL);
    }
    if (clipBranch.audioValve != nullptr) {
        gst_element_set_state(clipBranch.audioValve, GST_STATE_NULL);
    }
    if (clipBranch.queue != nullptr) {
        gst_element_set_state(clipBranch.queue, GST_STATE_NULL);
    }
    if (clipBranch.valve != nullptr) {
        gst_element_set_state(clipBranch.valve, GST_STATE_NULL);
    }
    if (clipBranch.parser != nullptr) {
        gst_element_set_state(clipBranch.parser, GST_STATE_NULL);
    }
    if (clipBranch.muxer != nullptr) {
        gst_element_set_state(clipBranch.muxer, GST_STATE_NULL);
    }
    if (clipBranch.sink != nullptr) {
        gst_element_set_state(clipBranch.sink, GST_STATE_NULL);
    }

    if (pipeline.videoEncodedTee != nullptr && clipBranch.teeSrcPad != nullptr) {
        gst_element_release_request_pad(pipeline.videoEncodedTee, clipBranch.teeSrcPad);
        gst_object_unref(clipBranch.teeSrcPad);
        clipBranch.teeSrcPad = nullptr;
    }

    if (pipeline.audioEncodedTee != nullptr && clipBranch.audioTeeSrcPad != nullptr) {
        gst_element_release_request_pad(pipeline.audioEncodedTee, clipBranch.audioTeeSrcPad);
        gst_object_unref(clipBranch.audioTeeSrcPad);
        clipBranch.audioTeeSrcPad = nullptr;
    }

    if (clipBranch.muxer != nullptr && clipBranch.audioMuxerSinkPad != nullptr) {
        gst_element_release_request_pad(clipBranch.muxer, clipBranch.audioMuxerSinkPad);
        gst_object_unref(clipBranch.audioMuxerSinkPad);
        clipBranch.audioMuxerSinkPad = nullptr;
    }

    if (pipeline.pipeline != nullptr) {
        auto* bin = GST_BIN(pipeline.pipeline);

        if (clipBranch.audioQueue != nullptr) {
            gst_bin_remove(bin, clipBranch.audioQueue);
            clipBranch.audioQueue = nullptr;
        }
        if (clipBranch.audioValve != nullptr) {
            gst_bin_remove(bin, clipBranch.audioValve);
            clipBranch.audioValve = nullptr;
        }
        if (clipBranch.queue != nullptr) {
            gst_bin_remove(bin, clipBranch.queue);
            clipBranch.queue = nullptr;
        }
        if (clipBranch.valve != nullptr) {
            gst_bin_remove(bin, clipBranch.valve);
            clipBranch.valve = nullptr;
        }
        if (clipBranch.parser != nullptr) {
            gst_bin_remove(bin, clipBranch.parser);
            clipBranch.parser = nullptr;
        }
        if (clipBranch.muxer != nullptr) {
            gst_bin_remove(bin, clipBranch.muxer);
            clipBranch.muxer = nullptr;
        }
        if (clipBranch.sink != nullptr) {
            gst_bin_remove(bin, clipBranch.sink);
            clipBranch.sink = nullptr;
        }
    }

    return RecordingResult{true, "Recording clip branch removed"};
}

void removeAllAvInspectionClipBranches(AvRecordingPipeline& pipeline) {
    while (!pipeline.inspectionClipBranches.empty()) {
        auto clipBranch = pipeline.inspectionClipBranches.begin();
        removeAvInspectionClipBranch(pipeline, *clipBranch->second);
        pipeline.inspectionClipBranches.erase(clipBranch);
    }
}

RecordingResult prepareVideoPath(
    const std::vector<AvRecordingVideoInput>& videoInputs,
    AvRecordingPipeline& pipeline
) {
    if (videoInputs.size() != 1) {
        return RecordingResult{false, "Recording expects exactly one AV video input"};
    }

    const auto& videoInput = videoInputs.front();
    std::string encoderName;
    pipeline.videoSource = gst_element_factory_make("intervideosrc", nullptr);
    pipeline.videoQueue = gst_element_factory_make("queue", nullptr);
    pipeline.videoRate = gst_element_factory_make("videorate", nullptr);
    pipeline.videoConvert = gst_element_factory_make("videoconvert", nullptr);
    pipeline.videoScale = gst_element_factory_make("videoscale", nullptr);
    pipeline.videoCapsFilter = gst_element_factory_make("capsfilter", nullptr);
    pipeline.videoEncoderCapsFilter = gst_element_factory_make("capsfilter", nullptr);
    pipeline.videoEncoder = createRecordingVideoEncoder(encoderName);
    pipeline.videoParser = gst_element_factory_make("h264parse", nullptr);
    pipeline.videoH264CapsFilter = gst_element_factory_make("capsfilter", nullptr);
    pipeline.videoEncodedTee = gst_element_factory_make("tee", nullptr);
    pipeline.videoMasterQueue = gst_element_factory_make("queue", nullptr);
    pipeline.videoOutputValve = gst_element_factory_make("valve", nullptr);
    pipeline.muxer = gst_element_factory_make("matroskamux", nullptr);
    pipeline.sink = gst_element_factory_make("filesink", nullptr);

    if (pipeline.videoSource == nullptr ||
        pipeline.videoQueue == nullptr ||
        pipeline.videoRate == nullptr ||
        pipeline.videoConvert == nullptr ||
        pipeline.videoScale == nullptr ||
        pipeline.videoCapsFilter == nullptr ||
        pipeline.videoEncoderCapsFilter == nullptr ||
        pipeline.videoEncoder == nullptr ||
        pipeline.videoParser == nullptr ||
        pipeline.videoH264CapsFilter == nullptr ||
        pipeline.videoEncodedTee == nullptr ||
        pipeline.videoMasterQueue == nullptr ||
        pipeline.videoOutputValve == nullptr ||
        pipeline.muxer == nullptr ||
        pipeline.sink == nullptr) {
        return RecordingResult{false, "Failed to create recording video pipeline elements or video encoder"};
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
    g_object_set(
        pipeline.videoMasterQueue,
        "max-size-buffers",
        0,
        "max-size-bytes",
        0,
        "max-size-time",
        0,
        nullptr
    );
    g_object_set(pipeline.videoOutputValve, "drop", TRUE, "drop-mode", 1, nullptr);
    g_object_set(pipeline.sink, "location", pipeline.outputPath.c_str(), "sync", FALSE, nullptr);

    if (encoderName == "qsvh264enc") {
        g_object_set(pipeline.videoEncoder, "target-usage", 4, nullptr);
    } else if (encoderName == "x264enc") {
        g_object_set(
            pipeline.videoEncoder,
            "speed-preset",
            1,
            "tune",
            0x00000004,
            "key-int-max",
            30,
            nullptr
        );
    }

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

    GstCaps* encoderInputCaps = gst_caps_new_simple(
        "video/x-raw",
        "format",
        G_TYPE_STRING,
        "NV12",
        nullptr
    );
    g_object_set(pipeline.videoEncoderCapsFilter, "caps", encoderInputCaps, nullptr);
    gst_caps_unref(encoderInputCaps);

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
        pipeline.videoEncoderCapsFilter,
        pipeline.videoEncoder,
        pipeline.videoParser,
        pipeline.videoH264CapsFilter,
        pipeline.videoEncodedTee,
        pipeline.videoMasterQueue,
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
            pipeline.videoEncoderCapsFilter,
            pipeline.videoEncoder,
            pipeline.videoParser,
            pipeline.videoH264CapsFilter,
            pipeline.videoEncodedTee,
            pipeline.videoMasterQueue,
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
    pipeline.audioEncodedTee = gst_element_factory_make("tee", nullptr);
    pipeline.audioMasterQueue = gst_element_factory_make("queue", nullptr);
    pipeline.audioOutputValve = gst_element_factory_make("valve", nullptr);

    if (pipeline.audioMixer == nullptr ||
        pipeline.audioMixerQueue == nullptr ||
        pipeline.audioMixerConvert == nullptr ||
        pipeline.audioMixerResample == nullptr ||
        pipeline.audioMixerCapsFilter == nullptr ||
        pipeline.audioEncoder == nullptr ||
        pipeline.audioEncodedTee == nullptr ||
        pipeline.audioMasterQueue == nullptr ||
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
    g_object_set(
        pipeline.audioMasterQueue,
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
        pipeline.audioEncodedTee,
        pipeline.audioMasterQueue,
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
            pipeline.audioEncodedTee,
            pipeline.audioMasterQueue,
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

    const auto videoPrepareResult = prepareVideoPath(videoInputs, pipeline);
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

bool hasSupportedRecordingVideoEncoder() {
    for (const char* candidate : kPreferredRecordingVideoEncoders) {
        GstElementFactory* factory = gst_element_factory_find(candidate);
        if (factory != nullptr) {
            gst_object_unref(factory);
            return true;
        }
    }

    return false;
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

RecordingResult startAvInspectionClip(
    AvRecordingPipeline& pipeline,
    int clipId,
    const std::string& outputPath
) {
    if (pipeline.pipeline == nullptr || pipeline.videoEncodedTee == nullptr) {
        return RecordingResult{false, "Recording pipeline is not running"};
    }

    if (clipId < 1) {
        return RecordingResult{false, "clipId must be a positive integer"};
    }

    const auto outputValidationResult = validateRecordingOutputPath(outputPath);
    if (!outputValidationResult.ok) {
        return outputValidationResult;
    }

    if (pipeline.inspectionClipBranches.find(clipId) != pipeline.inspectionClipBranches.end()) {
        return RecordingResult{false, "Clip recording is already running"};
    }

    auto clipBranch = std::make_unique<AvInspectionClipBranch>();
    clipBranch->clipId = clipId;
    clipBranch->outputPath = outputPath;
    clipBranch->queue = gst_element_factory_make("queue", nullptr);
    clipBranch->valve = gst_element_factory_make("valve", nullptr);
    clipBranch->parser = gst_element_factory_make("h264parse", nullptr);
    clipBranch->muxer = gst_element_factory_make("matroskamux", nullptr);
    clipBranch->sink = gst_element_factory_make("filesink", nullptr);
    clipBranch->audioActive = pipeline.audioActive && pipeline.audioEncodedTee != nullptr;

    if (clipBranch->audioActive) {
        clipBranch->audioQueue = gst_element_factory_make("queue", nullptr);
        clipBranch->audioValve = gst_element_factory_make("valve", nullptr);
    }

    if (clipBranch->queue == nullptr ||
        clipBranch->valve == nullptr ||
        clipBranch->parser == nullptr ||
        clipBranch->muxer == nullptr ||
        clipBranch->sink == nullptr ||
        (clipBranch->audioActive &&
         (clipBranch->audioQueue == nullptr || clipBranch->audioValve == nullptr))) {
        return RecordingResult{false, "Failed to create recording clip elements"};
    }

    g_object_set(clipBranch->queue, "max-size-buffers", 0, "max-size-bytes", 0, "max-size-time", 0, nullptr);
    g_object_set(clipBranch->valve, "drop", TRUE, "drop-mode", 1, nullptr);
    if (clipBranch->audioActive) {
        g_object_set(
            clipBranch->audioQueue,
            "max-size-buffers",
            0,
            "max-size-bytes",
            0,
            "max-size-time",
            0,
            nullptr
        );
        g_object_set(clipBranch->audioValve, "drop", TRUE, "drop-mode", 1, nullptr);
    }
    g_object_set(clipBranch->parser, "config-interval", -1, nullptr);
    g_object_set(clipBranch->muxer, "offset-to-zero", TRUE, nullptr);
    g_object_set(clipBranch->sink, "location", outputPath.c_str(), "sync", FALSE, nullptr);

    gst_bin_add_many(
        GST_BIN(pipeline.pipeline),
        clipBranch->queue,
        clipBranch->valve,
        clipBranch->parser,
        clipBranch->muxer,
        clipBranch->sink,
        nullptr
    );

    if (clipBranch->audioActive) {
        gst_bin_add_many(
            GST_BIN(pipeline.pipeline),
            clipBranch->audioQueue,
            clipBranch->audioValve,
            nullptr
        );
    }

    if (!gst_element_link_many(
            clipBranch->queue,
            clipBranch->valve,
            clipBranch->parser,
            clipBranch->muxer,
            clipBranch->sink,
            nullptr
        )) {
        removeAvInspectionClipBranch(pipeline, *clipBranch);
        return RecordingResult{false, "Failed to link recording clip branch"};
    }

    if (clipBranch->audioActive) {
        if (!gst_element_link_many(
                clipBranch->audioQueue,
                clipBranch->audioValve,
                nullptr
            )) {
            removeAvInspectionClipBranch(pipeline, *clipBranch);
            return RecordingResult{false, "Failed to link recording clip audio branch"};
        }

        GstPad* audioValveSrcPad = gst_element_get_static_pad(clipBranch->audioValve, "src");
        clipBranch->audioMuxerSinkPad = gst_element_request_pad_simple(clipBranch->muxer, "audio_%u");

        if (audioValveSrcPad == nullptr || clipBranch->audioMuxerSinkPad == nullptr) {
            if (audioValveSrcPad != nullptr) {
                gst_object_unref(audioValveSrcPad);
            }
            removeAvInspectionClipBranch(pipeline, *clipBranch);
            return RecordingResult{false, "Failed to prepare recording clip audio muxer pad"};
        }

        const GstPadLinkReturn audioMuxerLinkResult =
            gst_pad_link(audioValveSrcPad, clipBranch->audioMuxerSinkPad);
        gst_object_unref(audioValveSrcPad);

        if (audioMuxerLinkResult != GST_PAD_LINK_OK) {
            removeAvInspectionClipBranch(pipeline, *clipBranch);
            return RecordingResult{false, "Failed to connect recording clip audio to muxer"};
        }
    }

    GstPad* queueSinkPad = gst_element_get_static_pad(clipBranch->queue, "sink");
    clipBranch->teeSrcPad = gst_element_request_pad_simple(pipeline.videoEncodedTee, "src_%u");

    if (queueSinkPad == nullptr || clipBranch->teeSrcPad == nullptr) {
        if (queueSinkPad != nullptr) {
            gst_object_unref(queueSinkPad);
        }
        removeAvInspectionClipBranch(pipeline, *clipBranch);
        return RecordingResult{false, "Failed to prepare recording clip tee pads"};
    }

    const GstPadLinkReturn linkResult = gst_pad_link(clipBranch->teeSrcPad, queueSinkPad);
    gst_object_unref(queueSinkPad);
    if (linkResult != GST_PAD_LINK_OK) {
        removeAvInspectionClipBranch(pipeline, *clipBranch);
        return RecordingResult{false, "Failed to connect recording clip video branch"};
    }

    if (clipBranch->audioActive) {
        GstPad* audioQueueSinkPad = gst_element_get_static_pad(clipBranch->audioQueue, "sink");
        clipBranch->audioTeeSrcPad = gst_element_request_pad_simple(pipeline.audioEncodedTee, "src_%u");

        if (audioQueueSinkPad == nullptr || clipBranch->audioTeeSrcPad == nullptr) {
            if (audioQueueSinkPad != nullptr) {
                gst_object_unref(audioQueueSinkPad);
            }
            removeAvInspectionClipBranch(pipeline, *clipBranch);
            return RecordingResult{false, "Failed to prepare recording clip audio tee pads"};
        }

        const GstPadLinkReturn audioLinkResult =
            gst_pad_link(clipBranch->audioTeeSrcPad, audioQueueSinkPad);
        gst_object_unref(audioQueueSinkPad);
        if (audioLinkResult != GST_PAD_LINK_OK) {
            removeAvInspectionClipBranch(pipeline, *clipBranch);
            return RecordingResult{false, "Failed to connect recording clip audio branch"};
        }
    }

    GstPad* queueSrcPad = gst_element_get_static_pad(clipBranch->queue, "src");
    if (queueSrcPad == nullptr) {
        removeAvInspectionClipBranch(pipeline, *clipBranch);
        return RecordingResult{false, "Failed to inspect recording clip queue src pad"};
    }
    gst_pad_add_probe(
        queueSrcPad,
        GST_PAD_PROBE_TYPE_BUFFER,
        onClipBuffer,
        clipBranch.get(),
        nullptr
    );
    gst_object_unref(queueSrcPad);

    gst_element_sync_state_with_parent(clipBranch->sink);
    gst_element_sync_state_with_parent(clipBranch->muxer);
    gst_element_sync_state_with_parent(clipBranch->parser);
    gst_element_sync_state_with_parent(clipBranch->valve);
    gst_element_sync_state_with_parent(clipBranch->queue);
    if (clipBranch->audioActive) {
        gst_element_sync_state_with_parent(clipBranch->audioValve);
        gst_element_sync_state_with_parent(clipBranch->audioQueue);
    }

    const auto keyframeRequestResult = requestRecordingKeyframe(pipeline);
    if (!keyframeRequestResult.ok) {
        removeAvInspectionClipBranch(pipeline, *clipBranch);
        return keyframeRequestResult;
    }

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
                removeAvInspectionClipBranch(pipeline, *clipBranch);
                return RecordingResult{false, failureMessage};
            }
            case GST_MESSAGE_EOS:
                gst_message_unref(message);
                removeAvInspectionClipBranch(pipeline, *clipBranch);
                return RecordingResult{false, "Recording ended before clip data was available"};
            default:
                gst_message_unref(message);
                break;
            }
        }

        if (clipBranch->openedOnKeyframe) {
            pipeline.inspectionClipBranches[clipId] = std::move(clipBranch);
            return RecordingResult{true, "Recording clip started"};
        }
    }

    removeAvInspectionClipBranch(pipeline, *clipBranch);
    return RecordingResult{false, "Timed out waiting for recording clip keyframe"};
}

RecordingResult stopAvInspectionClip(AvRecordingPipeline& pipeline, int clipId) {
    if (clipId < 1) {
        return RecordingResult{false, "clipId must be a positive integer"};
    }

    const auto clipBranch = pipeline.inspectionClipBranches.find(clipId);
    if (clipBranch == pipeline.inspectionClipBranches.end()) {
        return RecordingResult{false, "Clip recording is not running"};
    }

    if (clipBranch->second->queue == nullptr || clipBranch->second->sink == nullptr) {
        return RecordingResult{false, "Recording clip branch is not available"};
    }

    RecordingEosProbeContext eosContext{false};
    GstPad* sinkPad = gst_element_get_static_pad(clipBranch->second->sink, "sink");
    if (sinkPad == nullptr) {
        removeAvInspectionClipBranch(pipeline, *clipBranch->second);
        pipeline.inspectionClipBranches.erase(clipBranch);
        return RecordingResult{false, "Failed to get recording clip sink pad"};
    }

    const gulong eosProbeId = gst_pad_add_probe(
        sinkPad,
        GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
        onRecordingSinkEos,
        &eosContext,
        nullptr
    );

    if (eosProbeId == 0) {
        gst_object_unref(sinkPad);
        removeAvInspectionClipBranch(pipeline, *clipBranch->second);
        pipeline.inspectionClipBranches.erase(clipBranch);
        return RecordingResult{false, "Failed to watch recording clip finalization"};
    }

    if (pipeline.videoEncodedTee != nullptr && clipBranch->second->teeSrcPad != nullptr) {
        gst_element_release_request_pad(pipeline.videoEncodedTee, clipBranch->second->teeSrcPad);
        gst_object_unref(clipBranch->second->teeSrcPad);
        clipBranch->second->teeSrcPad = nullptr;
    }

    if (pipeline.audioEncodedTee != nullptr && clipBranch->second->audioTeeSrcPad != nullptr) {
        gst_element_release_request_pad(pipeline.audioEncodedTee, clipBranch->second->audioTeeSrcPad);
        gst_object_unref(clipBranch->second->audioTeeSrcPad);
        clipBranch->second->audioTeeSrcPad = nullptr;
    }

    if (clipBranch->second->audioActive && clipBranch->second->audioQueue != nullptr) {
        GstPad* audioQueueSrcPad = gst_element_get_static_pad(clipBranch->second->audioQueue, "src");
        if (audioQueueSrcPad == nullptr) {
            gst_pad_remove_probe(sinkPad, eosProbeId);
            gst_object_unref(sinkPad);
            removeAvInspectionClipBranch(pipeline, *clipBranch->second);
            pipeline.inspectionClipBranches.erase(clipBranch);
            return RecordingResult{false, "Failed to get recording clip audio queue src pad"};
        }

        if (!gst_pad_push_event(audioQueueSrcPad, gst_event_new_eos())) {
            gst_object_unref(audioQueueSrcPad);
            gst_pad_remove_probe(sinkPad, eosProbeId);
            gst_object_unref(sinkPad);
            removeAvInspectionClipBranch(pipeline, *clipBranch->second);
            pipeline.inspectionClipBranches.erase(clipBranch);
            return RecordingResult{false, "Failed to send EOS to recording clip audio branch"};
        }

        gst_object_unref(audioQueueSrcPad);
    }

    GstPad* queueSrcPad = gst_element_get_static_pad(clipBranch->second->queue, "src");
    if (queueSrcPad == nullptr) {
        gst_pad_remove_probe(sinkPad, eosProbeId);
        gst_object_unref(sinkPad);
        removeAvInspectionClipBranch(pipeline, *clipBranch->second);
        pipeline.inspectionClipBranches.erase(clipBranch);
        return RecordingResult{false, "Failed to get recording clip queue src pad"};
    }

    if (!gst_pad_push_event(queueSrcPad, gst_event_new_eos())) {
        gst_object_unref(queueSrcPad);
        gst_pad_remove_probe(sinkPad, eosProbeId);
        gst_object_unref(sinkPad);
        removeAvInspectionClipBranch(pipeline, *clipBranch->second);
        pipeline.inspectionClipBranches.erase(clipBranch);
        return RecordingResult{false, "Failed to send EOS to recording clip branch"};
    }

    gst_object_unref(queueSrcPad);

    bool completed = false;
    {
        std::unique_lock<std::mutex> lock(eosContext.mutex);
        completed = eosContext.condition.wait_for(
            lock,
            kAvRecordingStopTimeout,
            [&eosContext]() { return eosContext.done; }
        );
    }

    if (!completed) {
        gst_pad_remove_probe(sinkPad, eosProbeId);
        gst_object_unref(sinkPad);
        removeAvInspectionClipBranch(pipeline, *clipBranch->second);
        pipeline.inspectionClipBranches.erase(clipBranch);
        return RecordingResult{false, "Timed out waiting for recording clip finalization"};
    }

    gst_object_unref(sinkPad);
    removeAvInspectionClipBranch(pipeline, *clipBranch->second);
    pipeline.inspectionClipBranches.erase(clipBranch);
    return RecordingResult{true, "Recording clip stopped"};
}

RecordingResult cancelAvInspectionClip(AvRecordingPipeline& pipeline, int clipId) {
    if (clipId < 1) {
        return RecordingResult{false, "clipId must be a positive integer"};
    }

    const auto clipBranch = pipeline.inspectionClipBranches.find(clipId);
    if (clipBranch == pipeline.inspectionClipBranches.end()) {
        return RecordingResult{false, "Clip recording is not running"};
    }

    removeAvInspectionClipBranch(pipeline, *clipBranch->second);
    pipeline.inspectionClipBranches.erase(clipBranch);
    return RecordingResult{true, "Recording clip cancelled"};
}

void removeAvRecordingPipeline(AvRecordingPipeline& pipeline) {
    removeAllAvInspectionClipBranches(pipeline);

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
