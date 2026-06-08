#pragma once

#include <gst/gst.h>

#include <string>

#include "mediaEngine/recording/shared/recording_types.h"

// Waits for EOS-driven recording finalization events and element-specific completion messages.

namespace travis::media_engine::recording {

[[nodiscard]] RecordingResult waitForFinalization(
    GstBus* bus,
    const std::string& successMessage
);
[[nodiscard]] RecordingResult waitForElementMessage(
    GstBus* bus,
    GstElement* target,
    GstMessageType targetMessageType,
    const std::string& successMessage,
    const std::string& timeoutMessage
);
GstPadProbeReturn onRecordingSinkEos(
    GstPad* pad,
    GstPadProbeInfo* info,
    gpointer userData
);

} // namespace travis::media_engine::recording
