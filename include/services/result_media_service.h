#pragma once

#include <QVector>

#include "data/repositories/result_image_repository.h"
#include "services/result_service.h"

// Service layer for result-image summary projections used by media-facing screens.

namespace travis::services {

class ResultMediaService {
public:
    explicit ResultMediaService(travis::data::repositories::ResultImageRepository resultImageRepository);

    // Returns lightweight image summary rows for one result.
    QVector<ResultImageSummary> listResultImageSummariesByResultId(qint64 resultId) const;

private:
    travis::data::repositories::ResultImageRepository resultImageRepository_;
};

} // namespace travis::services
