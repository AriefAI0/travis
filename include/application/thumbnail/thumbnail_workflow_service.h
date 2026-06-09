#pragma once

#include <QString>
#include <QVector>

#include "mediaEngine/thumbnail/thumbnail_generator.h"
#include "models/timeline_thumbnail.h"
#include "services/video_service.h"

// Coordinates thumbnail request validation with timeline thumbnail persistence.

namespace travis::application::thumbnail {

struct ThumbnailOutputRequest {
    qint64 timestampMs = 0;
    QString imagePath;
    qint64 width = 0;
    qint64 height = 0;
    qint64 sizeBytes = 0;
};

struct ThumbnailWorkflowResult {
    bool ok = false;
    QString message;
    QVector<travis::models::TimelineThumbnail> thumbnails;
};

class ThumbnailWorkflowService {
public:
    explicit ThumbnailWorkflowService(travis::services::VideoService& videoService);

    [[nodiscard]] ThumbnailWorkflowResult replaceMasterVideoTimelineThumbnails(
        qint64 masterVideoId,
        const QVector<ThumbnailOutputRequest>& outputs
    ) const;

private:
    travis::services::VideoService& videoService_;
    travis::media_engine::thumbnail::ThumbnailGenerator thumbnailGenerator_;
};

} // namespace travis::application::thumbnail
