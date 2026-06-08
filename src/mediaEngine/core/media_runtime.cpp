#include "mediaEngine/core/media_runtime.h"

#include <gst/gst.h>

#include <array>
#include <utility>

// Owns the shared GStreamer bootstrap used by embedded media-engine modules.

namespace travis::media_engine::core {

namespace {

constexpr std::array<const char*, 6> kRequiredElements = {
    "ndisrc",
    "ndisrcdemux",
    "mfvideosrc",
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

    const bool hasD3d11PreviewSink = hasElementFactory("qml6d3d11sink");
    const bool hasGlPreviewSink = hasElementFactory("qml6glsink");

    if (!hasD3d11PreviewSink && !hasGlPreviewSink) {
        missingPlugins.append(QStringLiteral("qml6d3d11sink|qml6glsink"));
    }

    if (hasGlPreviewSink) {
        if (!hasElementFactory("glupload")) {
            missingPlugins.append(QStringLiteral("glupload"));
        }
        if (!hasElementFactory("glcolorconvert")) {
            missingPlugins.append(QStringLiteral("glcolorconvert"));
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
