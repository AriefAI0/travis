#pragma once

#include <gst/gst.h>

// Provides small helpers for inspecting GStreamer caps on dynamic pads.

namespace travis::media_engine::pipeline {

[[nodiscard]] bool padHasMediaTypePrefix(GstPad* pad, const char* mediaTypePrefix);

} // namespace travis::media_engine::pipeline
