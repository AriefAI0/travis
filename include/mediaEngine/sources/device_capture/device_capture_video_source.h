#pragma once

#include <gst/gst.h>

#include <string>

// Builds the device-capture source chain used by the embedded media engine.

namespace travis::media_engine::sources::device_capture {

struct DeviceCaptureVideoSource {
    GstElement* source = nullptr;
};

[[nodiscard]] DeviceCaptureVideoSource createDeviceCaptureVideoSource(
    const std::string& deviceName,
    const std::string& devicePath,
    const std::string& sourceElement
);
[[nodiscard]] bool isDeviceCaptureVideoSourceValid(const DeviceCaptureVideoSource& source);
void addDeviceCaptureVideoSourceToBin(GstBin* bin, const DeviceCaptureVideoSource& source);
[[nodiscard]] bool linkDeviceCaptureVideoSourceToQueue(
    const DeviceCaptureVideoSource& source,
    GstElement* queue
);

} // namespace travis::media_engine::sources::device_capture
