#include "mediaEngine/thumbnail/thumbnail_generator.h"

#include <QFileInfo>

// Keeps thumbnail request validation centralized in the media engine layer.

namespace travis::media_engine::thumbnail {

ThumbnailResult ThumbnailGenerator::validateRequest(const ThumbnailRequest& request) const {
    if (request.mediaFilePath.trimmed().isEmpty()) {
        return ThumbnailResult{false, QStringLiteral("Thumbnail media file path is required")};
    }

    const QFileInfo mediaFile(request.mediaFilePath);
    if (!mediaFile.exists() || !mediaFile.isFile()) {
        return ThumbnailResult{false, QStringLiteral("Thumbnail media file does not exist")};
    }

    if (request.outputImagePath.trimmed().isEmpty()) {
        return ThumbnailResult{false, QStringLiteral("Thumbnail output image path is required")};
    }

    if (request.timestampMs < 0) {
        return ThumbnailResult{false, QStringLiteral("Thumbnail timestamp must not be negative")};
    }

    if (request.width < 1 || request.height < 1) {
        return ThumbnailResult{false, QStringLiteral("Thumbnail dimensions must be positive")};
    }

    return ThumbnailResult{true, QStringLiteral("Thumbnail request is valid")};
}

} // namespace travis::media_engine::thumbnail
