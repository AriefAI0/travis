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
    // Returns the default inspection type names seeded for a new database.
    static const QVector<QString>& defaultInspectionTypeNames();

    ResultService(
        travis::data::repositories::InspectionTypeRepository inspectionTypeRepository,
        travis::data::repositories::ResultRepository resultRepository,
        travis::data::repositories::ResultImageRepository resultImageRepository
    );

    // Creates an inspection type after validating the name.
    std::optional<travis::models::InspectionType> createInspectionType(
        const travis::models::InspectionTypeCreateInput& input
    ) const;
    // Inserts the default inspection types when they are missing.
    void ensureDefaultInspectionTypes() const;
    // Returns all inspection types.
    QVector<travis::models::InspectionType> listInspectionTypes() const;
    // Returns one inspection type by id.
    std::optional<travis::models::InspectionType> getInspectionTypeById(qint64 inspectionTypeId) const;
    // Returns one inspection type by name.
    std::optional<travis::models::InspectionType> getInspectionTypeByName(const QString& name) const;
    // Updates one inspection type after validating the name when provided.
    std::optional<travis::models::InspectionType> updateInspectionType(
        qint64 inspectionTypeId,
        const travis::models::InspectionTypeUpdateInput& input
    ) const;
    // Deletes one inspection type by id.
    bool deleteInspectionType(qint64 inspectionTypeId) const;

    // Creates a result after normalizing optional text fields.
    std::optional<travis::models::Result> createResult(const travis::models::ResultCreateInput& input) const;
    // Returns all results.
    QVector<travis::models::Result> listResults() const;
    // Returns results for one session-item row.
    QVector<travis::models::Result> listResultsBySessionItemId(qint64 sessionItemId) const;
    // Returns one result by id.
    std::optional<travis::models::Result> getResultById(qint64 resultId) const;
    // Updates one result after normalizing provided fields.
    std::optional<travis::models::Result> updateResult(
        qint64 resultId,
        const travis::models::ResultUpdateInput& input
    ) const;
    // Deletes one result by id.
    bool deleteResult(qint64 resultId) const;

    // Creates a result-image row after validating the raw image path.
    std::optional<travis::models::ResultImage> createResultImage(
        const travis::models::ResultImageCreateInput& input
    ) const;
    // Returns all result-image rows.
    QVector<travis::models::ResultImage> listResultImages() const;
    // Returns result-image rows for one result.
    QVector<travis::models::ResultImage> listResultImagesByResultId(qint64 resultId) const;
    // Returns lightweight image summary rows for one result.
    QVector<ResultImageSummary> listResultImageSummariesByResultId(qint64 resultId) const;
    // Returns one result-image row by id.
    std::optional<travis::models::ResultImage> getResultImageById(qint64 imageId) const;
    // Updates one result-image row after normalizing provided fields.
    std::optional<travis::models::ResultImage> updateResultImage(
        qint64 imageId,
        const travis::models::ResultImageUpdateInput& input
    ) const;
    // Deletes one result-image row by id.
    bool deleteResultImage(qint64 imageId) const;

private:
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository_;
    travis::data::repositories::ResultRepository resultRepository_;
    travis::data::repositories::ResultImageRepository resultImageRepository_;
};

} // namespace travis::services
