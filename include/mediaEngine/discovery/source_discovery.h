#pragma once

#include <QString>
#include <QVector>

// Discovers capture sources through the bundled GStreamer runtime.

namespace travis::media_engine::discovery {

struct DiscoveredMediaSource {
    QString id;
    QString kind;
    QString name;
    QString devicePath;
    QString sourceElement;
    QString urlAddress;
};

struct SourceDiscoveryResult {
    bool ok = false;
    QString message;
    QVector<DiscoveredMediaSource> sources;
};

class SourceDiscovery {
public:
    [[nodiscard]] SourceDiscoveryResult listDeviceCaptureSources() const;
    [[nodiscard]] SourceDiscoveryResult listAudioCaptureSources() const;

private:
    [[nodiscard]] SourceDiscoveryResult runDeviceMonitor(const QStringList& classes) const;
};

} // namespace travis::media_engine::discovery
