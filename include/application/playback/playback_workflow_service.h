#pragma once

#include <optional>

#include "models/master_video.h"
#include "models/video_clip.h"
#include "services/result_media_service.h"
#include "services/video_service.h"

// Coordinates playback-facing validation and joins for master videos and clips.

namespace travis::application::playback {

struct PlaybackMasterVideo {
    travis::models::MasterVideo masterVideo;
    QVector<travis::models::TimelineThumbnail> thumbnails;
};

struct PlaybackClip {
    travis::services::VideoClipPlayback clipPlayback;
    QVector<travis::services::ResultImageSummary> resultImages;
};

class PlaybackWorkflowService {
public:
    PlaybackWorkflowService(
        travis::services::VideoService& videoService,
        travis::services::ResultMediaService& resultMediaService
    );

    [[nodiscard]] std::optional<PlaybackMasterVideo> openMasterVideo(qint64 masterVideoId) const;
    [[nodiscard]] std::optional<PlaybackClip> openVideoClip(qint64 clipId) const;

private:
    travis::services::VideoService& videoService_;
    travis::services::ResultMediaService& resultMediaService_;
};

} // namespace travis::application::playback
