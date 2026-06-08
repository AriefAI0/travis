#pragma once

#include <gst/gst.h>

#include <string>

// Extracts readable failure messages from GStreamer bus errors.

namespace travis::media_engine::pipeline {

[[nodiscard]] std::string readGstErrorMessage(GstMessage* message);

} // namespace travis::media_engine::pipeline
