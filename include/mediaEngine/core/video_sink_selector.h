#pragma once

#include <gst/gst.h>

// Selects the best available preview sink for the current Windows runtime.

namespace travis::media_engine::core {

GstElement* createPreviewVideoSink();

} // namespace travis::media_engine::core
