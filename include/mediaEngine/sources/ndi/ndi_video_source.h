#pragma once

#include <gst/gst.h>

#include <string>

// Builds the NDI source chain used by the embedded media engine.

namespace travis::media_engine::sources::ndi {

struct NdiVideoSource {
    GstElement* source = nullptr;
    GstElement* demux = nullptr;
    GstElement* decoder = nullptr;
    GstElement* discardQueue = nullptr;
    GstElement* discardSink = nullptr;
};

[[nodiscard]] NdiVideoSource createNdiVideoSource(
    const std::string& sourceName,
    const std::string& urlAddress
);
[[nodiscard]] bool isNdiVideoSourceValid(const NdiVideoSource& source);
void addNdiVideoSourceToBin(GstBin* bin, const NdiVideoSource& source);
[[nodiscard]] bool linkNdiVideoSource(const NdiVideoSource& source);
void connectNdiVideoSourceToQueue(
    const NdiVideoSource& source,
    GstElement* queue,
    bool& linkedVideoPad
);

} // namespace travis::media_engine::sources::ndi
