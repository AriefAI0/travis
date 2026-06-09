#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include "mediaEngine/discovery/source_discovery.h"

// Exposes media source discovery results to QML without exposing GStreamer details.

namespace travis::ui::viewmodels {

class SourceDiscoveryViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList videoSources READ videoSources NOTIFY videoSourcesChanged)
    Q_PROPERTY(QVariantList audioSources READ audioSources NOTIFY audioSourcesChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit SourceDiscoveryViewModel(QObject* parent = nullptr);

    [[nodiscard]] QVariantList videoSources() const;
    [[nodiscard]] QVariantList audioSources() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool refreshVideoSources();
    Q_INVOKABLE bool refreshAudioSources();
    Q_INVOKABLE bool refreshAll();

signals:
    void videoSourcesChanged();
    void audioSourcesChanged();
    void loadingChanged();
    void lastErrorChanged();

private:
    [[nodiscard]] QVariantList toVariantList(
        const QVector<travis::media_engine::discovery::DiscoveredMediaSource>& sources
    ) const;
    void setLoading(bool loading);
    void setLastError(const QString& lastError);

    travis::media_engine::discovery::SourceDiscovery sourceDiscovery_;
    QVector<travis::media_engine::discovery::DiscoveredMediaSource> videoSources_;
    QVector<travis::media_engine::discovery::DiscoveredMediaSource> audioSources_;
    bool loading_ = false;
    QString lastError_;
};

} // namespace travis::ui::viewmodels
