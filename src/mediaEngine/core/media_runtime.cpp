#include "mediaEngine/core/media_runtime.h"

#include <gst/gst.h>

#include <array>

// Owns the shared GStreamer bootstrap used by embedded media-engine modules.

namespace travis::media_engine::core {

namespace {

constexpr std::array<const char*, 9> kRequiredElements = {
    "ndisrc",
    "ndisrcdemux",
    "mfvideosrc",
    "qml6glsink",
    "glupload",
    "glcolorconvert",
    "qsvh264enc",
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
