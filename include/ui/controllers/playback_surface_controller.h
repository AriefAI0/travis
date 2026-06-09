#pragma once

#include <QObject>
#include <QString>

#include <QPointer>

#include "mediaEngine/core/media_runtime.h"
#include "mediaEngine/playback/playback_engine.h"

// Exposes native Qt/QML playback surface controls to QML.

namespace travis::ui::controllers {

class PlaybackSurfaceController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject* playbackItem READ playbackItem WRITE setPlaybackItem NOTIFY playbackItemChanged)
    Q_PROPERTY(bool playbackActive READ playbackActive NOTIFY playbackActiveChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit PlaybackSurfaceController(QObject* parent = nullptr);

    QObject* playbackItem() const;
    void setPlaybackItem(QObject* playbackItem);

    [[nodiscard]] bool playbackActive() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool startPlayback(const QString& filePath);
    Q_INVOKABLE void stopPlayback();

signals:
    void playbackItemChanged();
    void playbackActiveChanged();
    void lastErrorChanged();

private:
    void setLastError(const QString& lastError);

    travis::media_engine::core::MediaRuntime mediaRuntime_;
    travis::media_engine::playback::PlaybackEngine playbackEngine_;
    QPointer<QObject> playbackItem_;
    QString lastError_;
};

} // namespace travis::ui::controllers
