#pragma once

#include <gst/gst.h>

// Selects the native Windows video sink strategy for the embedded media engine.

namespace travis::media_engine::core {

enum class PreviewVideoSinkKind {
    D3d11Video,
};

struct PreviewVideoSinkSelection {
    PreviewVideoSinkKind kind;
    GstElement* sink = nullptr;
};

[[nodiscard]] PreviewVideoSinkSelection createPreviewVideoSink();

} // namespace travis::media_engine::core
