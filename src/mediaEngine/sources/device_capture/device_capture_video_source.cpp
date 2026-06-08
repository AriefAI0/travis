#include "mediaEngine/sources/device_capture/device_capture_video_source.h"

// Constructs the Windows capture source element and attaches the selected device properties.

namespace travis::media_engine::sources::device_capture {

namespace {

constexpr const char* kMediaFoundationVideoSource = "mfvideosrc";

} // namespace

DeviceCaptureVideoSource createDeviceCaptureVideoSource(
    const std::string& deviceName,
    const std::string& devicePath,
    const std::string& sourceElement
) {
    const std::string effectiveSourceElement =
        sourceElement.empty() ? kMediaFoundationVideoSource : sourceElement;

    DeviceCaptureVideoSource source{
        gst_element_factory_make(effectiveSourceElement.c_str(), nullptr),
    };

    if (!isDeviceCaptureVideoSourceValid(source)) {
        return source;
    }

    if (!devicePath.empty()) {
        g_object_set(source.source, "device-path", devicePath.c_str(), nullptr);
    } else if (!deviceName.empty()) {
        g_object_set(source.source, "device-name", deviceName.c_str(), nullptr);
    }

    return source;
}

bool isDeviceCaptureVideoSourceValid(const DeviceCaptureVideoSource& source) {
    return source.source != nullptr;
}

void addDeviceCaptureVideoSourceToBin(GstBin* bin, const DeviceCaptureVideoSource& source) {
    gst_bin_add(GST_BIN(bin), source.source);
}

bool linkDeviceCaptureVideoSourceToQueue(
    const DeviceCaptureVideoSource& source,
    GstElement* queue
) {
    return gst_element_link(source.source, queue);
}

} // namespace travis::media_engine::sources::device_capture
