#include "application/playback/playback_workflow_service.h"

// Validates persisted playback state before media is opened by the UI layer.

namespace travis::application::playback {

PlaybackWorkflowService::PlaybackWorkflowService(
    travis::services::VideoService& videoService,
    travis::services::ResultMediaService& resultMediaService
)
    : videoService_(videoService)
    , resultMediaService_(resultMediaService) {}

std::optional<PlaybackMasterVideo> PlaybackWorkflowService::openMasterVideo(qint64 masterVideoId) const {
    const auto masterVideo = videoService_.getMasterVideoById(masterVideoId);
    if (!masterVideo.has_value()) {
        return std::nullopt;
    }

    return PlaybackMasterVideo{
        .masterVideo = *masterVideo,
        .thumbnails = videoService_.listMasterVideoTimelineThumbnailsByMasterVideoId(masterVideoId),
    };
}

std::optional<PlaybackClip> PlaybackWorkflowService::openVideoClip(qint64 clipId) const {
    const auto clipPlayback = videoService_.getVideoClipPlaybackById(clipId);
    if (!clipPlayback.has_value()) {
        return std::nullopt;
    }

    return PlaybackClip{
        .clipPlayback = *clipPlayback,
        .resultImages = resultMediaService_.listResultImageSummariesByResultId(clipPlayback->resultId),
    };
}

} // namespace travis::application::playback
