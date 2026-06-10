#include "mediaEngine/core/video_sink_selector.h"

#include <iostream>

// Selects the preferred native Windows preview sink for the current runtime.

namespace travis::media_engine::core {

PreviewVideoSinkSelection createPreviewVideoSink() {
    if (GstElement* d3d11Sink = gst_element_factory_make("d3d11videosink", nullptr); d3d11Sink != nullptr) {
        std::cerr << "[PreviewSink] Selected d3d11videosink\n";
        return PreviewVideoSinkSelection{
            .kind = PreviewVideoSinkKind::D3d11Video,
            .sink = d3d11Sink,
        };
    }

    std::cerr << "[PreviewSink] d3d11videosink unavailable. Check the GStreamer d3d11 plugin runtime.\n";
    return PreviewVideoSinkSelection{
        .kind = PreviewVideoSinkKind::D3d11Video,
        .sink = nullptr,
    };
}

} // namespace travis::media_engine::core
