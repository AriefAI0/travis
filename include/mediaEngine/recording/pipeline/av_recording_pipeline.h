#pragma once

#include <gst/gst.h>

#include <chrono>
#include <string>
#include <vector>

#include "mediaEngine/recording/shared/recording_types.h"
#include "mediaEngine/session/media_source_session.h"

// Defines the dedicated AV recording pipeline used by the embedded recording engine.

namespace travis::media_engine::recording {

struct AvRecordingVideoInput {
    std::string sourceKind;
    std::string sourceName;
    std::string sourceElement;
    std::string bridgeChannel;
    int width = 0;
    int height = 0;
};

struct AvRecordingAudioInputBranch {
    GstElement* source = nullptr;
    GstElement* queue = nullptr;
    GstElement* convert = nullptr;
    GstElement* resample = nullptr;
    GstElement* capsFilter = nullptr;
    GstPad* mixerSinkPad = nullptr;
    bool receivedBuffer = false;
};

struct AvRecordingPipeline {
    std::string recordingId;
    std::string outputPath;
    travis::media_engine::session::MediaSourceSessionManager* sessionManager = nullptr;
    std::vector<travis::media_engine::session::MediaSourceSession*> sourceSessions;
    GstElement* pipeline = nullptr;
    GstBus* bus = nullptr;
    GstElement* videoSource = nullptr;
    GstElement* videoQueue = nullptr;
    GstElement* videoRate = nullptr;
    GstElement* videoConvert = nullptr;
    GstElement* videoScale = nullptr;
    GstElement* videoCapsFilter = nullptr;
    GstElement* videoEncoderCapsFilter = nullptr;
    GstElement* videoEncoder = nullptr;
    GstElement* videoParser = nullptr;
    GstElement* videoH264CapsFilter = nullptr;
    GstElement* videoOutputValve = nullptr;
    GstElement* audioMixer = nullptr;
    GstElement* audioMixerQueue = nullptr;
    GstElement* audioMixerConvert = nullptr;
    GstElement* audioMixerResample = nullptr;
    GstElement* audioMixerCapsFilter = nullptr;
    GstElement* audioEncoder = nullptr;
    GstElement* audioOutputValve = nullptr;
    GstElement* muxer = nullptr;
    GstElement* sink = nullptr;
    std::vector<AvRecordingAudioInputBranch> audioInputBranches;
    bool audioActive = false;
    bool receivedVideoBuffer = false;
    bool receivedAudioBuffer = false;
    std::chrono::steady_clock::time_point startedAt;
};

[[nodiscard]] bool hasSupportedRecordingVideoEncoder();

[[nodiscard]] RecordingResult startAvRecordingPipeline(
    const std::string& recordingId,
    const std::vector<AvRecordingVideoInput>& videoInputs,
    const std::vector<RecordingAudioInput>& audioInputs,
    const std::string& outputPath,
    travis::media_engine::session::MediaSourceSessionManager* sessionManager,
    const std::vector<travis::media_engine::session::MediaSourceSession*>& sourceSessions,
    AvRecordingPipeline& pipeline
);
[[nodiscard]] RecordingResult stopAvRecordingPipeline(AvRecordingPipeline& pipeline);
void removeAvRecordingPipeline(AvRecordingPipeline& pipeline);

} // namespace travis::media_engine::recording
