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

    // Creates a master video after validating the time range and text fields.
    std::optional<travis::models::MasterVideo> createMasterVideo(
        const travis::models::MasterVideoCreateInput& input
    ) const;
    // Returns all master videos.
    QVector<travis::models::MasterVideo> listMasterVideos() const;
    // Returns master videos for one session.
    QVector<travis::models::MasterVideo> listMasterVideosBySessionId(qint64 sessionId) const;
    // Returns master videos for one status value.
    QVector<travis::models::MasterVideo> listMasterVideosByStatus(const QString& status) const;
    // Returns one master video by id.
    std::optional<travis::models::MasterVideo> getMasterVideoById(qint64 masterVideoId) const;
    // Updates one master video after validating the merged time range and text fields.
    std::optional<travis::models::MasterVideo> updateMasterVideo(
        qint64 masterVideoId,
        const travis::models::MasterVideoUpdateInput& input
    ) const;
    // Deletes one master video by id.
    bool deleteMasterVideo(qint64 masterVideoId) const;

    // Creates a video clip after validating clip offsets.
    std::optional<travis::models::VideoClip> createVideoClip(
        const travis::models::VideoClipCreateInput& input
    ) const;
    // Starts a clip in recording state with no end offset yet.
    std::optional<travis::models::VideoClip> startVideoClip(
        qint64 resultId,
        qint64 masterVideoId,
        qint64 startOffsetMs
    ) const;
    // Returns all video clips.
    QVector<travis::models::VideoClip> listVideoClips() const;
    // Returns video clips for one result.
    QVector<travis::models::VideoClip> listVideoClipsByResultId(qint64 resultId) const;
    // Returns video clips for one master video.
    QVector<travis::models::VideoClip> listVideoClipsByMasterVideoId(qint64 masterVideoId) const;
    // Returns video clips for one status value.
    QVector<travis::models::VideoClip> listVideoClipsByStatus(const QString& status) const;
    // Returns one video clip by id.
    std::optional<travis::models::VideoClip> getVideoClipById(qint64 clipId) const;
    // Builds playback data by combining the clip row with its parent master video.
    std::optional<VideoClipPlayback> getVideoClipPlaybackById(qint64 clipId) const;
    // Returns the unfinished clip for one result when it exists.
    std::optional<travis::models::VideoClip> getActiveVideoClipByResultId(qint64 resultId) const;
    // Updates one video clip after validating merged offsets and text fields.
    std::optional<travis::models::VideoClip> updateVideoClip(
        qint64 clipId,
        const travis::models::VideoClipUpdateInput& input
    ) const;
    // Completes a clip by setting its end offset and final media paths.
    std::optional<travis::models::VideoClip> completeVideoClip(
        qint64 clipId,
        qint64 endOffsetMs,
        const std::optional<QString>& clipFileUrl,
        const std::optional<QString>& thumbnailUrl
    ) const;
    // Deletes one video clip by id.
    bool deleteVideoClip(qint64 clipId) const;

    // Replaces all timeline thumbnails for one master video in a single operation.
    QVector<travis::models::TimelineThumbnail> replaceMasterVideoTimelineThumbnails(
        qint64 masterVideoId,
        const travis::models::TimelineThumbnailCreateBatch& thumbnails
    ) const;
    // Returns timeline thumbnails for one master video.
    QVector<travis::models::TimelineThumbnail> listMasterVideoTimelineThumbnailsByMasterVideoId(
        qint64 masterVideoId
    ) const;

private:
    travis::data::repositories::MasterVideoRepository masterVideoRepository_;
    travis::data::repositories::VideoClipRepository videoClipRepository_;
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository_;
};

} // namespace travis::services
