#include "mediaEngine/core/video_sink_selector.h"

#include <iostream>

// Selects the preferred Qt/QML preview sink for the current runtime.

namespace travis::media_engine::core {

PreviewVideoSinkSelection createPreviewVideoSink() {
    if (GstElement* d3d11Sink = gst_element_factory_make("qml6d3d11sink", nullptr); d3d11Sink != nullptr) {
        std::cerr << "[PreviewSink] Selected qml6d3d11sink\n";
        return PreviewVideoSinkSelection{
            .kind = PreviewVideoSinkKind::Qml6D3d11,
            .sink = d3d11Sink,
        };
    }

    std::cerr << "[PreviewSink] qml6d3d11sink unavailable, trying qml6glsink\n";

    GstElement* glSink = gst_element_factory_make("qml6glsink", nullptr);
    if (glSink != nullptr) {
        std::cerr << "[PreviewSink] Selected qml6glsink\n";
    } else {
        std::cerr << "[PreviewSink] qml6glsink unavailable — both Qt QML sinks missing."
                     " Check GST_PLUGIN_PATH and that libgstqml6.dll is present in the GStreamer plugin directory.\n";
    }

    return PreviewVideoSinkSelection{
        .kind = PreviewVideoSinkKind::Qml6Gl,
        .sink = glSink,
    };
}

} // namespace travis::media_engine::core
