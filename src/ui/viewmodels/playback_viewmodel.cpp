#include "ui/viewmodels/playback_viewmodel.h"

// Adapts playback workflow results into simple Qt properties for QML binding.

namespace travis::ui::viewmodels {

PlaybackViewModel::PlaybackViewModel(
    travis::application::playback::PlaybackWorkflowService& playbackWorkflowService,
    QObject* parent
)
    : QObject(parent)
    , playbackWorkflowService_(playbackWorkflowService) {}

qint64 PlaybackViewModel::selectedMasterVideoId() const {
    return selectedMasterVideoId_;
}

QString PlaybackViewModel::selectedMasterVideoPath() const {
    return selectedMasterVideoPath_;
}

QStringList PlaybackViewModel::timelineThumbnailPaths() const {
    return timelineThumbnailPaths_;
}

qint64 PlaybackViewModel::selectedClipId() const {
    return selectedClipId_;
}

QString PlaybackViewModel::selectedClipPath() const {
    return selectedClipPath_;
}

QStringList PlaybackViewModel::resultImagePaths() const {
    return resultImagePaths_;
}

QString PlaybackViewModel::statusMessage() const {
    return statusMessage_;
}

QString PlaybackViewModel::lastError() const {
    return lastError_;
}

bool PlaybackViewModel::openMasterVideo(qint64 masterVideoId) {
    const auto playback = playbackWorkflowService_.openMasterVideo(masterVideoId);
    if (!playback.has_value()) {
        setLastError(QStringLiteral("Master video not found"));
        return false;
    }

    selectedMasterVideoId_ = playback->masterVideo.masterVideoId;
    selectedMasterVideoPath_ = playback->masterVideo.fileUrl;
    timelineThumbnailPaths_.clear();
    for (const auto& thumbnail : playback->thumbnails) {
        timelineThumbnailPaths_.append(thumbnail.imagePath);
    }

    emit selectedMasterVideoChanged();
    setLastError(QString{});
    setStatusMessage(QStringLiteral("Master video opened"));
    return true;
}

bool PlaybackViewModel::openVideoClip(qint64 clipId) {
    const auto playback = playbackWorkflowService_.openVideoClip(clipId);
    if (!playback.has_value()) {
        setLastError(QStringLiteral("Video clip not found"));
        return false;
    }

    selectedClipId_ = playback->clipPlayback.clipId;
    selectedClipPath_ = playback->clipPlayback.clipFileUrl.value_or(QString{});
    resultImagePaths_.clear();
    for (const auto& resultImage : playback->resultImages) {
        resultImagePaths_.append(resultImage.rawUrl);
    }

    emit selectedClipChanged();
    setLastError(QString{});
    setStatusMessage(QStringLiteral("Video clip opened"));
    return true;
}

void PlaybackViewModel::clearSelection() {
    selectedMasterVideoId_ = 0;
    selectedMasterVideoPath_.clear();
    timelineThumbnailPaths_.clear();
    selectedClipId_ = 0;
    selectedClipPath_.clear();
    resultImagePaths_.clear();

    emit selectedMasterVideoChanged();
    emit selectedClipChanged();
    setStatusMessage(QString{});
    setLastError(QString{});
}

void PlaybackViewModel::setStatusMessage(const QString& statusMessage) {
    if (statusMessage_ == statusMessage) {
        return;
    }

    statusMessage_ = statusMessage;
    emit statusMessageChanged();
}

void PlaybackViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

} // namespace travis::ui::viewmodels
