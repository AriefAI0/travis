#pragma once

#include <gst/gst.h>

// Selects the Qt/QML-native preview sink strategy for the embedded preview engine.

namespace travis::media_engine::core {

enum class PreviewVideoSinkKind {
    Qml6D3d11,
    Qml6Gl,
};

struct PreviewVideoSinkSelection {
    PreviewVideoSinkKind kind;
    GstElement* sink = nullptr;
};

[[nodiscard]] PreviewVideoSinkSelection createPreviewVideoSink();

} // namespace travis::media_engine::core
