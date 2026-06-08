#pragma once

#include <QStringList>

// Initializes GStreamer once per process and validates the required engine plugins.

namespace travis::media_engine::core {

struct GStreamerHealthCheckResult {
    bool ok = false;
    QStringList missingPlugins;
};

class MediaRuntime {
public:
    MediaRuntime();

    // Returns the current plugin availability required by the embedded media engine.
    [[nodiscard]] GStreamerHealthCheckResult healthCheck() const;

private:
    [[nodiscard]] bool hasElementFactory(const char* elementName) const;
};

} // namespace travis::media_engine::core
