#pragma once

#include <gst/gst.h>

// Creates the QML-native preview sink used by the embedded preview engine.

namespace travis::media_engine::core {

GstElement* createPreviewVideoSink();

} // namespace travis::media_engine::core
