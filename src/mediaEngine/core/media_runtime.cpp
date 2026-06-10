#include "mediaEngine/core/media_runtime.h"

#include <gst/gst.h>

#include <array>
#include <utility>

// Owns the shared GStreamer bootstrap used by embedded media-engine modules.

namespace travis::media_engine::core {

namespace {

constexpr std::array<const char*, 5> kRequiredElements = {
    "ndisrc",
    "ndisrcdemux",
    "mfvideosrc",
    "h264parse",
    "matroskamux",
};

} // namespace

MediaRuntime::MediaRuntime() {
    gst_init(nullptr, nullptr);
}

GStreamerHealthCheckResult MediaRuntime::healthCheck() const {
    QStringList missingPlugins;

    for (const char* elementName : kRequiredElements) {
        if (!hasElementFactory(elementName)) {
            missingPlugins.append(QString::fromUtf8(elementName));
        }
    }

    if (!hasElementFactory("d3d11videosink")) {
        missingPlugins.append(QStringLiteral("d3d11videosink"));
    }

    const bool hasRecordingEncoder =
        hasElementFactory("qsvh264enc") ||
        hasElementFactory("mfh264enc") ||
        hasElementFactory("x264enc");
    if (!hasRecordingEncoder) {
        missingPlugins.append(QStringLiteral("qsvh264enc|mfh264enc|x264enc"));
    }

    return GStreamerHealthCheckResult{
        .ok = missingPlugins.isEmpty(),
        .missingPlugins = missingPlugins,
    };
}

bool MediaRuntime::hasElementFactory(const char* elementName) const {
    GstElementFactory* factory = gst_element_factory_find(elementName);

    if (factory == nullptr) {
        return false;
    }

    gst_object_unref(factory);
    return true;
}

} // namespace travis::media_engine::core
