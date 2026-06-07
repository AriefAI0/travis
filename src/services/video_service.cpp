#include "services/video_service.h"

#include <stdexcept>

namespace travis::services {

namespace {

void validateMasterVideoTimeRange(qint64 startEpoch, const std::optional<qint64>& endEpoch) {
    if (startEpoch < 0) {
        throw std::runtime_error("Master video startEpoch must be non-negative");
    }

    if (endEpoch.has_value() && *endEpoch <= startEpoch) {
        throw std::runtime_error("Master video endEpoch must be greater than startEpoch");
    }
}

void validateVideoClipOffsets(qint64 startOffsetMs, const std::optional<qint64>& endOffsetMs) {
    if (startOffsetMs < 0) {
        throw std::runtime_error("Video clip startOffsetMs must be non-negative");
    }

    if (endOffsetMs.has_value() && *endOffsetMs <= startOffsetMs) {
        throw std::runtime_error("Video clip endOffsetMs must be greater than startOffsetMs");
    }
}

QString normalizeRequiredText(const QString& value, const char* fieldName) {
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        throw std::runtime_error(fieldName);
    }

    return trimmedValue;
}

std::optional<QString> normalizeOptionalText(const std::optional<QString>& value) {
    if (!value.has_value()) {
        return std::nullopt;
    }

    const QString trimmedValue = value->trimmed();
    if (trimmedValue.isEmpty()) {
        return std::nullopt;
    }

    return trimmedValue;
}

void validateThumbnailBatch(
    qint64 masterVideoId,
    const travis::models::TimelineThumbnailCreateBatch& thumbnails
) {
    for (const travis::models::TimelineThumbnailCreateInput& thumbnail : thumbnails) {
        if (thumbnail.masterVideoId != masterVideoId) {
            throw std::runtime_error("Thumbnail masterVideoId must match target master video");
        }
        if (thumbnail.timestampMs < 0) {
            throw std::runtime_error("Thumbnail timestampMs must be non-negative");
        }
        if (thumbnail.width < 1 || thumbnail.height < 1) {
            throw std::runtime_error("Thumbnail width and height must be positive");
        }
        if (thumbnail.sizeBytes < 0) {
            throw std::runtime_error("Thumbnail sizeBytes must be non-negative");
        }
        normalizeRequiredText(thumbnail.imagePath, "Thumbnail imagePath is required");
    }
}

} // namespace

VideoService::VideoService(
    travis::data::repositories::MasterVideoRepository masterVideoRepository,
    travis::data::repositories::VideoClipRepository videoClipRepository,
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository
)
    : masterVideoRepository_(std::move(masterVideoRepository))
    , videoClipRepository_(std::move(videoClipRepository))
    , timelineThumbnailRepository_(std::move(timelineThumbnailRepository)) {}

std::optional<travis::models::MasterVideo> VideoService::createMasterVideo(
    const travis::models::MasterVideoCreateInput& input
) const {
    validateMasterVideoTimeRange(input.startEpoch, input.endEpoch);

    return masterVideoRepository_.create({
        .sessionId = input.sessionId,
        .fileUrl = normalizeRequiredText(input.fileUrl, "Master video fileUrl is required"),
        .thumbnailUrl = normalizeOptionalText(input.thumbnailUrl),
        .startEpoch = input.startEpoch,
        .endEpoch = input.endEpoch,
        .status = input.status.value_or(QStringLiteral("completed")),
        .sourceName = normalizeOptionalText(input.sourceName),
    });
}

QVector<travis::models::MasterVideo> VideoService::listMasterVideos() const {
    return masterVideoRepository_.listAll();
}

QVector<travis::models::MasterVideo> VideoService::listMasterVideosBySessionId(qint64 sessionId) const {
    return masterVideoRepository_.listBySessionId(sessionId);
}

QVector<travis::models::MasterVideo> VideoService::listMasterVideosByStatus(const QString& status) const {
    return masterVideoRepository_.listByStatus(normalizeRequiredText(status, "Master video status is required"));
}

std::optional<travis::models::MasterVideo> VideoService::getMasterVideoById(qint64 masterVideoId) const {
    return masterVideoRepository_.findById(masterVideoId);
}

std::optional<travis::models::MasterVideo> VideoService::updateMasterVideo(
    qint64 masterVideoId,
    const travis::models::MasterVideoUpdateInput& input
) const {
    const std::optional<travis::models::MasterVideo> existingMasterVideo = masterVideoRepository_.findById(masterVideoId);
    if (!existingMasterVideo.has_value()) {
        return std::nullopt;
    }

    validateMasterVideoTimeRange(
        input.startEpoch.value_or(existingMasterVideo->startEpoch),
        input.endEpoch.has_value() ? input.endEpoch : existingMasterVideo->endEpoch
    );

    travis::models::MasterVideoUpdateInput normalizedInput = input;
    if (input.fileUrl.has_value()) {
        normalizedInput.fileUrl = normalizeRequiredText(*input.fileUrl, "Master video fileUrl is required");
    }
    if (input.thumbnailUrl.has_value()) {
        normalizedInput.thumbnailUrl = normalizeOptionalText(input.thumbnailUrl);
    }
    if (input.sourceName.has_value()) {
        normalizedInput.sourceName = normalizeOptionalText(input.sourceName);
    }
    if (input.status.has_value()) {
        normalizedInput.status = normalizeRequiredText(*input.status, "Master video status is required");
    }

    return masterVideoRepository_.updateById(masterVideoId, normalizedInput);
}

bool VideoService::deleteMasterVideo(qint64 masterVideoId) const {
    return masterVideoRepository_.removeById(masterVideoId);
}

std::optional<travis::models::VideoClip> VideoService::createVideoClip(
    const travis::models::VideoClipCreateInput& input
) const {
    validateVideoClipOffsets(input.startOffsetMs, input.endOffsetMs);

    return videoClipRepository_.create({
        .resultId = input.resultId,
        .masterVideoId = input.masterVideoId,
        .startOffsetMs = input.startOffsetMs,
        .endOffsetMs = input.endOffsetMs,
        .clipFileUrl = normalizeOptionalText(input.clipFileUrl),
        .thumbnailUrl = normalizeOptionalText(input.thumbnailUrl),
        .status = input.status.value_or(QStringLiteral("completed")),
    });
}

std::optional<travis::models::VideoClip> VideoService::startVideoClip(
    qint64 resultId,
    qint64 masterVideoId,
    qint64 startOffsetMs
) const {
    return createVideoClip({
        .resultId = resultId,
        .masterVideoId = masterVideoId,
        .startOffsetMs = startOffsetMs,
        .endOffsetMs = std::nullopt,
        .clipFileUrl = std::nullopt,
        .thumbnailUrl = std::nullopt,
        .status = QStringLiteral("recording"),
    });
}

QVector<travis::models::VideoClip> VideoService::listVideoClips() const {
    return videoClipRepository_.listAll();
}

QVector<travis::models::VideoClip> VideoService::listVideoClipsByResultId(qint64 resultId) const {
    return videoClipRepository_.listByResultId(resultId);
}

QVector<travis::models::VideoClip> VideoService::listVideoClipsByMasterVideoId(qint64 masterVideoId) const {
    return videoClipRepository_.listByMasterVideoId(masterVideoId);
}

QVector<travis::models::VideoClip> VideoService::listVideoClipsByStatus(const QString& status) const {
    return videoClipRepository_.listByStatus(normalizeRequiredText(status, "Video clip status is required"));
}

std::optional<travis::models::VideoClip> VideoService::getVideoClipById(qint64 clipId) const {
    return videoClipRepository_.findById(clipId);
}

std::optional<VideoClipPlayback> VideoService::getVideoClipPlaybackById(qint64 clipId) const {
    const std::optional<travis::models::VideoClip> clip = videoClipRepository_.findById(clipId);
    if (!clip.has_value()) {
        return std::nullopt;
    }

    const std::optional<travis::models::MasterVideo> masterVideo = masterVideoRepository_.findById(clip->masterVideoId);
    if (!masterVideo.has_value()) {
        return std::nullopt;
    }

    const std::optional<qint64> durationMs =
        clip->endOffsetMs.has_value() ? std::optional<qint64>(*clip->endOffsetMs - clip->startOffsetMs) : std::nullopt;
    const std::optional<qint64> endEpochMs =
        clip->endOffsetMs.has_value() ? std::optional<qint64>(masterVideo->startEpoch * 1000 + *clip->endOffsetMs) : std::nullopt;

    return VideoClipPlayback{
        .clipId = clip->clipId,
        .resultId = clip->resultId,
        .masterVideoId = clip->masterVideoId,
        .fileUrl = masterVideo->fileUrl,
        .masterVideoStartEpoch = masterVideo->startEpoch,
        .masterVideoEndEpoch = masterVideo->endEpoch,
        .startOffsetMs = clip->startOffsetMs,
        .endOffsetMs = clip->endOffsetMs,
        .clipFileUrl = clip->clipFileUrl,
        .thumbnailUrl = clip->thumbnailUrl,
        .durationMs = durationMs,
        .startEpochMs = masterVideo->startEpoch * 1000 + clip->startOffsetMs,
        .endEpochMs = endEpochMs,
    };
}

std::optional<travis::models::VideoClip> VideoService::getActiveVideoClipByResultId(qint64 resultId) const {
    return videoClipRepository_.findActiveByResultId(resultId);
}

std::optional<travis::models::VideoClip> VideoService::updateVideoClip(
    qint64 clipId,
    const travis::models::VideoClipUpdateInput& input
) const {
    const std::optional<travis::models::VideoClip> existingVideoClip = videoClipRepository_.findById(clipId);
    if (!existingVideoClip.has_value()) {
        return std::nullopt;
    }

    validateVideoClipOffsets(
        input.startOffsetMs.value_or(existingVideoClip->startOffsetMs),
        input.endOffsetMs.has_value() ? input.endOffsetMs : existingVideoClip->endOffsetMs
    );

    travis::models::VideoClipUpdateInput normalizedInput = input;
    if (input.clipFileUrl.has_value()) {
        normalizedInput.clipFileUrl = normalizeOptionalText(input.clipFileUrl);
    }
    if (input.thumbnailUrl.has_value()) {
        normalizedInput.thumbnailUrl = normalizeOptionalText(input.thumbnailUrl);
    }
    if (input.status.has_value()) {
        normalizedInput.status = normalizeRequiredText(*input.status, "Video clip status is required");
    }

    return videoClipRepository_.updateById(clipId, normalizedInput);
}

std::optional<travis::models::VideoClip> VideoService::completeVideoClip(
    qint64 clipId,
    qint64 endOffsetMs,
    const std::optional<QString>& clipFileUrl,
    const std::optional<QString>& thumbnailUrl
) const {
    const std::optional<travis::models::VideoClip> existingVideoClip = videoClipRepository_.findById(clipId);
    if (!existingVideoClip.has_value()) {
        return std::nullopt;
    }

    validateVideoClipOffsets(existingVideoClip->startOffsetMs, endOffsetMs);

    return videoClipRepository_.updateById(clipId, {
        .endOffsetMs = endOffsetMs,
        .clipFileUrl = normalizeOptionalText(clipFileUrl),
        .thumbnailUrl = normalizeOptionalText(thumbnailUrl),
        .status = QStringLiteral("completed"),
    });
}

bool VideoService::deleteVideoClip(qint64 clipId) const {
    return videoClipRepository_.removeById(clipId);
}

QVector<travis::models::TimelineThumbnail> VideoService::replaceMasterVideoTimelineThumbnails(
    qint64 masterVideoId,
    const travis::models::TimelineThumbnailCreateBatch& thumbnails
) const {
    if (!masterVideoRepository_.findById(masterVideoId).has_value()) {
        throw std::runtime_error("Master video does not exist");
    }

    validateThumbnailBatch(masterVideoId, thumbnails);

    if (!timelineThumbnailRepository_.removeByMasterVideoId(masterVideoId)) {
        return {};
    }

    return timelineThumbnailRepository_.createMany(thumbnails);
}

QVector<travis::models::TimelineThumbnail> VideoService::listMasterVideoTimelineThumbnailsByMasterVideoId(
    qint64 masterVideoId
) const {
    return timelineThumbnailRepository_.listByMasterVideoId(masterVideoId);
}

} // namespace travis::services
