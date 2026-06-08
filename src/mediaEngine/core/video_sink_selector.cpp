#include "mediaEngine/core/video_sink_selector.h"

#include <array>

// Chooses the first supported preview sink from the engine priority list.

namespace travis::media_engine::core {

namespace {

constexpr std::array<const char*, 4> kPreviewSinkPriority = {
    "d3d11videosink",
    "d3d12videosink",
    "glimagesink",
    "autovideosink",
};

} // namespace

GstElement* createPreviewVideoSink() {
    for (const char* sinkName : kPreviewSinkPriority) {
        GstElement* sink = gst_element_factory_make(sinkName, nullptr);
        if (sink != nullptr) {
            return sink;
        }
    }

    return nullptr;
}

} // namespace travis::media_engine::core
