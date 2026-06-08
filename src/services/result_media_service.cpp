#include "services/result_media_service.h"

namespace travis::services {

ResultMediaService::ResultMediaService(
    travis::data::repositories::ResultImageRepository resultImageRepository
)
    : resultImageRepository_(std::move(resultImageRepository)) {}

QVector<ResultImageSummary> ResultMediaService::listResultImageSummariesByResultId(qint64 resultId) const {
    const QVector<travis::models::ResultImage> images = resultImageRepository_.listByResultId(resultId);
    QVector<ResultImageSummary> summaries;
    summaries.reserve(images.size());

    for (const travis::models::ResultImage& image : images) {
        summaries.append({
            .imageId = image.imageId,
            .rawUrl = image.rawUrl,
            .annotatedUrl = image.annotatedUrl,
            .remarks = image.remarks,
        });
    }

    return summaries;
}

} // namespace travis::services
