#pragma once

#include <QString>
#include <QVector>

#include <optional>

#include "data/repositories/master_video_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "models/master_video.h"
#include "models/timeline_thumbnail.h"
#include "models/video_clip.h"

// Service layer for master videos, video clips, and timeline thumbnails.

namespace travis::services {

struct VideoClipPlayback {
    qint64 clipId = 0;
    qint64 resultId = 0;
    qint64 masterVideoId = 0;
    QString fileUrl;
    qint64 masterVideoStartEpoch = 0;
    std::optional<qint64> masterVideoEndEpoch;
    qint64 startOffsetMs = 0;
    std::optional<qint64> endOffsetMs;
    std::optional<QString> clipFileUrl;
    std::optional<QString> thumbnailUrl;
    std::optional<qint64> durationMs;
    qint64 startEpochMs = 0;
    std::optional<qint64> endEpochMs;
};

class VideoService {
public:
    VideoService(
        travis::data::repositories::MasterVideoRepository masterVideoRepository,
        travis::data::repositories::VideoClipRepository videoClipRepository,
        travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository
    );

    std::optional<travis::models::MasterVideo> createMasterVideo(
        const travis::models::MasterVideoCreateInput& input
    ) const;
    QVector<travis::models::MasterVideo> listMasterVideos() const;
    QVector<travis::models::MasterVideo> listMasterVideosBySessionId(qint64 sessionId) const;
    QVector<travis::models::MasterVideo> listMasterVideosByStatus(const QString& status) const;
    std::optional<travis::models::MasterVideo> getMasterVideoById(qint64 masterVideoId) const;
    std::optional<travis::models::MasterVideo> updateMasterVideo(
        qint64 masterVideoId,
        const travis::models::MasterVideoUpdateInput& input
    ) const;
    bool deleteMasterVideo(qint64 masterVideoId) const;

    std::optional<travis::models::VideoClip> createVideoClip(
        const travis::models::VideoClipCreateInput& input
    ) const;
    std::optional<travis::models::VideoClip> startVideoClip(
        qint64 resultId,
        qint64 masterVideoId,
        qint64 startOffsetMs
    ) const;
    QVector<travis::models::VideoClip> listVideoClips() const;
    QVector<travis::models::VideoClip> listVideoClipsByResultId(qint64 resultId) const;
    QVector<travis::models::VideoClip> listVideoClipsByMasterVideoId(qint64 masterVideoId) const;
    QVector<travis::models::VideoClip> listVideoClipsByStatus(const QString& status) const;
    std::optional<travis::models::VideoClip> getVideoClipById(qint64 clipId) const;
    std::optional<VideoClipPlayback> getVideoClipPlaybackById(qint64 clipId) const;
    std::optional<travis::models::VideoClip> getActiveVideoClipByResultId(qint64 resultId) const;
    std::optional<travis::models::VideoClip> updateVideoClip(
        qint64 clipId,
        const travis::models::VideoClipUpdateInput& input
    ) const;
    std::optional<travis::models::VideoClip> completeVideoClip(
        qint64 clipId,
        qint64 endOffsetMs,
        const std::optional<QString>& clipFileUrl,
        const std::optional<QString>& thumbnailUrl
    ) const;
    bool deleteVideoClip(qint64 clipId) const;

    QVector<travis::models::TimelineThumbnail> replaceMasterVideoTimelineThumbnails(
        qint64 masterVideoId,
        const travis::models::TimelineThumbnailCreateBatch& thumbnails
    ) const;
    QVector<travis::models::TimelineThumbnail> listMasterVideoTimelineThumbnailsByMasterVideoId(
        qint64 masterVideoId
    ) const;

private:
    travis::data::repositories::MasterVideoRepository masterVideoRepository_;
    travis::data::repositories::VideoClipRepository videoClipRepository_;
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository_;
};

} // namespace travis::services
