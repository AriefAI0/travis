#pragma once

#include <QString>
#include <QVector>

#include <optional>

#include "data/repositories/inspection_type_repository.h"
#include "data/repositories/result_image_repository.h"
#include "data/repositories/result_repository.h"
#include "models/inspection_type.h"
#include "models/result.h"
#include "models/result_image.h"

// Service layer for inspection types, results, and result images.

namespace travis::services {

struct ResultImageSummary {
    qint64 imageId = 0;
    QString rawUrl;
    std::optional<QString> annotatedUrl;
    std::optional<QString> remarks;
};

class ResultService {
public:
    static const QVector<QString>& defaultInspectionTypeNames();

    ResultService(
        travis::data::repositories::InspectionTypeRepository inspectionTypeRepository,
        travis::data::repositories::ResultRepository resultRepository,
        travis::data::repositories::ResultImageRepository resultImageRepository
    );

    std::optional<travis::models::InspectionType> createInspectionType(
        const travis::models::InspectionTypeCreateInput& input
    ) const;
    void ensureDefaultInspectionTypes() const;
    QVector<travis::models::InspectionType> listInspectionTypes() const;
    std::optional<travis::models::InspectionType> getInspectionTypeById(qint64 inspectionTypeId) const;
    std::optional<travis::models::InspectionType> getInspectionTypeByName(const QString& name) const;
    std::optional<travis::models::InspectionType> updateInspectionType(
        qint64 inspectionTypeId,
        const travis::models::InspectionTypeUpdateInput& input
    ) const;
    bool deleteInspectionType(qint64 inspectionTypeId) const;

    std::optional<travis::models::Result> createResult(const travis::models::ResultCreateInput& input) const;
    QVector<travis::models::Result> listResults() const;
    QVector<travis::models::Result> listResultsBySessionItemId(qint64 sessionItemId) const;
    std::optional<travis::models::Result> getResultById(qint64 resultId) const;
    std::optional<travis::models::Result> updateResult(
        qint64 resultId,
        const travis::models::ResultUpdateInput& input
    ) const;
    bool deleteResult(qint64 resultId) const;

    std::optional<travis::models::ResultImage> createResultImage(
        const travis::models::ResultImageCreateInput& input
    ) const;
    QVector<travis::models::ResultImage> listResultImages() const;
    QVector<travis::models::ResultImage> listResultImagesByResultId(qint64 resultId) const;
    QVector<ResultImageSummary> listResultImageSummariesByResultId(qint64 resultId) const;
    std::optional<travis::models::ResultImage> getResultImageById(qint64 imageId) const;
    std::optional<travis::models::ResultImage> updateResultImage(
        qint64 imageId,
        const travis::models::ResultImageUpdateInput& input
    ) const;
    bool deleteResultImage(qint64 imageId) const;

private:
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository_;
    travis::data::repositories::ResultRepository resultRepository_;
    travis::data::repositories::ResultImageRepository resultImageRepository_;
};

} // namespace travis::services
