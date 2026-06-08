#include "mediaEngine/core/video_sink_selector.h"

// Selects the preferred Qt/QML preview sink for the current runtime.

namespace travis::media_engine::core {

PreviewVideoSinkSelection createPreviewVideoSink() {
    if (GstElement* d3d11Sink = gst_element_factory_make("qml6d3d11sink", nullptr); d3d11Sink != nullptr) {
        return PreviewVideoSinkSelection{
            .kind = PreviewVideoSinkKind::Qml6D3d11,
            .sink = d3d11Sink,
        };
    }

    return PreviewVideoSinkSelection{
        .kind = PreviewVideoSinkKind::Qml6Gl,
        .sink = gst_element_factory_make("qml6glsink", nullptr),
    };
}

} // namespace travis::media_engine::core
