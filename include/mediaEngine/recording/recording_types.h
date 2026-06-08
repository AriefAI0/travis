#pragma once

#include <gst/gst.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include "mediaEngine/session/media_source_session.h"

// Defines shared recording result and input types for the embedded recording engine.

namespace travis::media_engine::recording {

struct RecordingResult {
    bool ok = false;
    std::string message;
};

struct RecordingPositionResult {
    bool ok = false;
    std::string message;
    gint64 positionMs = 0;
};

struct RecordingAudioInput {
    std::string deviceName;
    std::string devicePath;
    std::string sourceElement;
};

struct RecordingVideoInput {
    std::string sourceKind;
    std::string sourceName;
    std::string urlAddress;
    std::string devicePath;
    std::string sourceElement;
    int xpos = 0;
    int ypos = 0;
    int width = 0;
    int height = 0;
    double alpha = 1.0;
    int zorder = 0;
};

struct RecordingEosProbeContext {
    bool done = false;
    std::mutex mutex;
    std::condition_variable condition;
};

struct PreparedRecordingSource {
    std::string sourceName;
    travis::media_engine::session::MediaSourceSession* session = nullptr;
};

} // namespace travis::media_engine::recording
