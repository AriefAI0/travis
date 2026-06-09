#pragma once

#include <QString>

// Validates thumbnail extraction requests before frame-grab pipeline work is executed.

namespace travis::media_engine::thumbnail {

struct ThumbnailRequest {
    QString mediaFilePath;
    QString outputImagePath;
    qint64 timestampMs = 0;
    int width = 0;
    int height = 0;
};

struct ThumbnailResult {
    bool ok = false;
    QString message;
};

class ThumbnailGenerator {
public:
    [[nodiscard]] ThumbnailResult validateRequest(const ThumbnailRequest& request) const;
};

} // namespace travis::media_engine::thumbnail
