#include "application/thumbnail/thumbnail_workflow_service.h"

#include <exception>

// Persists thumbnail outputs only after media-layer request validation succeeds.

namespace travis::application::thumbnail {

ThumbnailWorkflowService::ThumbnailWorkflowService(travis::services::VideoService& videoService)
    : videoService_(videoService) {}

ThumbnailWorkflowResult ThumbnailWorkflowService::replaceMasterVideoTimelineThumbnails(
    qint64 masterVideoId,
    const QVector<ThumbnailOutputRequest>& outputs
) const {
    if (masterVideoId <= 0) {
        return ThumbnailWorkflowResult{false, QStringLiteral("masterVideoId is required"), {}};
    }

    if (outputs.isEmpty()) {
        return ThumbnailWorkflowResult{false, QStringLiteral("At least one thumbnail output is required"), {}};
    }

    const auto masterVideo = videoService_.getMasterVideoById(masterVideoId);
    if (!masterVideo.has_value()) {
        return ThumbnailWorkflowResult{false, QStringLiteral("Master video not found"), {}};
    }

    travis::models::TimelineThumbnailCreateBatch thumbnailBatch;
    thumbnailBatch.reserve(outputs.size());

    for (const auto& output : outputs) {
        const auto validation = thumbnailGenerator_.validateRequest({
            .mediaFilePath = masterVideo->fileUrl,
            .outputImagePath = output.imagePath,
            .timestampMs = output.timestampMs,
            .width = static_cast<int>(output.width),
            .height = static_cast<int>(output.height),
        });
        if (!validation.ok) {
            return ThumbnailWorkflowResult{false, validation.message, {}};
        }

        thumbnailBatch.append({
            .masterVideoId = masterVideoId,
            .timestampMs = output.timestampMs,
            .imagePath = output.imagePath,
            .width = output.width,
            .height = output.height,
            .sizeBytes = output.sizeBytes,
        });
    }

    try {
        return ThumbnailWorkflowResult{
            true,
            QStringLiteral("Timeline thumbnails replaced"),
            videoService_.replaceMasterVideoTimelineThumbnails(masterVideoId, thumbnailBatch),
        };
    } catch (const std::exception& exception) {
        return ThumbnailWorkflowResult{false, QString::fromUtf8(exception.what()), {}};
    }
}

} // namespace travis::application::thumbnail
