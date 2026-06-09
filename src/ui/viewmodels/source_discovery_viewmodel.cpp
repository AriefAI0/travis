#include "ui/viewmodels/source_discovery_viewmodel.h"

#include <QVariantMap>

// Converts native source discovery results into stable QML list data.

namespace travis::ui::viewmodels {

SourceDiscoveryViewModel::SourceDiscoveryViewModel(QObject* parent)
    : QObject(parent) {}

QVariantList SourceDiscoveryViewModel::videoSources() const {
    return toVariantList(videoSources_);
}

QVariantList SourceDiscoveryViewModel::audioSources() const {
    return toVariantList(audioSources_);
}

bool SourceDiscoveryViewModel::loading() const {
    return loading_;
}

QString SourceDiscoveryViewModel::lastError() const {
    return lastError_;
}

bool SourceDiscoveryViewModel::refreshVideoSources() {
    setLoading(true);
    const auto result = sourceDiscovery_.listDeviceCaptureSources();
    setLoading(false);

    if (!result.ok) {
        setLastError(result.message);
        return false;
    }

    videoSources_ = result.sources;
    emit videoSourcesChanged();
    setLastError(QString{});
    return true;
}

bool SourceDiscoveryViewModel::refreshAudioSources() {
    setLoading(true);
    const auto result = sourceDiscovery_.listAudioCaptureSources();
    setLoading(false);

    if (!result.ok) {
        setLastError(result.message);
        return false;
    }

    audioSources_ = result.sources;
    emit audioSourcesChanged();
    setLastError(QString{});
    return true;
}

bool SourceDiscoveryViewModel::refreshAll() {
    const bool videoOk = refreshVideoSources();
    const bool audioOk = refreshAudioSources();
    return videoOk && audioOk;
}

QVariantList SourceDiscoveryViewModel::toVariantList(
    const QVector<travis::media_engine::discovery::DiscoveredMediaSource>& sources
) const {
    QVariantList sourceVariants;
    sourceVariants.reserve(sources.size());

    for (const auto& source : sources) {
        sourceVariants.append(QVariantMap{
            {QStringLiteral("id"), source.id},
            {QStringLiteral("kind"), source.kind},
            {QStringLiteral("name"), source.name},
            {QStringLiteral("devicePath"), source.devicePath},
            {QStringLiteral("sourceElement"), source.sourceElement},
            {QStringLiteral("urlAddress"), source.urlAddress},
        });
    }

    return sourceVariants;
}

void SourceDiscoveryViewModel::setLoading(bool loading) {
    if (loading_ == loading) {
        return;
    }

    loading_ = loading;
    emit loadingChanged();
}

void SourceDiscoveryViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

} // namespace travis::ui::viewmodels
