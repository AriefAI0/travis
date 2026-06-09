#include "ui/controllers/playback_surface_controller.h"

#include <QQuickItem>

// Keeps rendered playback controls separate from persisted playback metadata.

namespace travis::ui::controllers {

PlaybackSurfaceController::PlaybackSurfaceController(QObject* parent)
    : QObject(parent)
    , mediaRuntime_()
    , playbackEngine_() {
    const auto healthCheck = mediaRuntime_.healthCheck();
    if (!healthCheck.ok) {
        setLastError(
            QStringLiteral("Missing GStreamer plugins: %1").arg(healthCheck.missingPlugins.join(QStringLiteral(", ")))
        );
    }
}

QObject* PlaybackSurfaceController::playbackItem() const {
    return playbackItem_;
}

void PlaybackSurfaceController::setPlaybackItem(QObject* playbackItem) {
    if (playbackItem_ == playbackItem) {
        return;
    }

    playbackItem_ = playbackItem;
    emit playbackItemChanged();
}

bool PlaybackSurfaceController::playbackActive() const {
    return playbackEngine_.isRunning();
}

QString PlaybackSurfaceController::lastError() const {
    return lastError_;
}

bool PlaybackSurfaceController::startPlayback(const QString& filePath) {
    auto* quickItem = qobject_cast<QQuickItem*>(playbackItem_.data());
    if (quickItem == nullptr) {
        setLastError(QStringLiteral("playbackItem must be a QQuickItem"));
        return false;
    }

    const auto result = playbackEngine_.startFilePlayback(filePath.toStdString(), quickItem);
    if (!result.ok) {
        setLastError(QString::fromStdString(result.message));
        emit playbackActiveChanged();
        return false;
    }

    setLastError(QString{});
    emit playbackActiveChanged();
    return true;
}

void PlaybackSurfaceController::stopPlayback() {
    const auto result = playbackEngine_.stopPlayback();
    if (!result.ok) {
        setLastError(QString::fromStdString(result.message));
    } else {
        setLastError(QString{});
    }

    emit playbackActiveChanged();
}

void PlaybackSurfaceController::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

} // namespace travis::ui::controllers
