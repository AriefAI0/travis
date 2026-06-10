#pragma once

#include <gst/gst.h>

#include <string>

class QQuickItem;

// Applies Qt Quick item geometry to native GStreamer video overlay sinks.

namespace travis::media_engine::core {

struct NativeVideoOverlayResult {
    bool ok = false;
    std::string message;
};

[[nodiscard]] NativeVideoOverlayResult syncNativeVideoOverlayGeometry(
    GstElement* sink,
    QQuickItem* targetItem
);

} // namespace travis::media_engine::core
