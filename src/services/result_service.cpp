#include "services/result_service.h"

#include <stdexcept>

namespace travis::services {

namespace {

const QVector<QString> kDefaultInspectionTypeNames = {
    QStringLiteral("GVI"),
    QStringLiteral("CVI"),
    QStringLiteral("MGI"),
    QStringLiteral("CP"),
    QStringLiteral("FMD"),
};

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

} // namespace

const QVector<QString>& ResultService::defaultInspectionTypeNames() {
    return kDefaultInspectionTypeNames;
}

ResultService::ResultService(
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository,
    travis::data::repositories::ResultRepository resultRepository,
    travis::data::repositories::ResultImageRepository resultImageRepository
)
    : inspectionTypeRepository_(std::move(inspectionTypeRepository))
    , resultRepository_(std::move(resultRepository))
    , resultImageRepository_(std::move(resultImageRepository)) {}

std::optional<travis::models::InspectionType> ResultService::createInspectionType(
    const travis::models::InspectionTypeCreateInput& input
) const {
    return inspectionTypeRepository_.create({
        .name = normalizeRequiredText(input.name, "Inspection type name is required"),
    });
}

void ResultService::ensureDefaultInspectionTypes() const {
    for (const QString& name : defaultInspectionTypeNames()) {
        inspectionTypeRepository_.createIfMissing({ .name = name });
    }
}

QVector<travis::models::InspectionType> ResultService::listInspectionTypes() const {
    return inspectionTypeRepository_.listAll();
}

std::optional<travis::models::InspectionType> ResultService::getInspectionTypeById(qint64 inspectionTypeId) const {
    return inspectionTypeRepository_.findById(inspectionTypeId);
}

std::optional<travis::models::InspectionType> ResultService::getInspectionTypeByName(const QString& name) const {
    return inspectionTypeRepository_.findByName(
        normalizeRequiredText(name, "Inspection type name is required")
    );
}

std::optional<travis::models::InspectionType> ResultService::updateInspectionType(
    qint64 inspectionTypeId,
    const travis::models::InspectionTypeUpdateInput& input
) const {
    travis::models::InspectionTypeUpdateInput normalizedInput;

    if (input.name.has_value()) {
        normalizedInput.name = normalizeRequiredText(*input.name, "Inspection type name is required");
    }

    return inspectionTypeRepository_.updateById(inspectionTypeId, normalizedInput);
}

bool ResultService::deleteInspectionType(qint64 inspectionTypeId) const {
    return inspectionTypeRepository_.removeById(inspectionTypeId);
}

std::optional<travis::models::Result> ResultService::createResult(
    const travis::models::ResultCreateInput& input
) const {
    return resultRepository_.create({
        .sessionItemId = input.sessionItemId,
        .inspectionTypeId = input.inspectionTypeId,
        .executionUnitId = input.executionUnitId,
        .toolingId = input.toolingId,
        .value = normalizeOptionalText(input.value),
        .status = normalizeOptionalText(input.status),
        .remarks = normalizeOptionalText(input.remarks),
        .configSnapshot = normalizeOptionalText(input.configSnapshot),
    });
}

QVector<travis::models::Result> ResultService::listResults() const {
    return resultRepository_.listAll();
}

QVector<travis::models::Result> ResultService::listResultsBySessionItemId(qint64 sessionItemId) const {
    return resultRepository_.listBySessionItemId(sessionItemId);
}

std::optional<travis::models::Result> ResultService::getResultById(qint64 resultId) const {
    return resultRepository_.findById(resultId);
}

std::optional<travis::models::Result> ResultService::updateResult(
    qint64 resultId,
    const travis::models::ResultUpdateInput& input
) const {
    travis::models::ResultUpdateInput normalizedInput;

    if (input.sessionItemId.has_value()) {
        normalizedInput.sessionItemId = input.sessionItemId;
    }
    if (input.inspectionTypeId.has_value()) {
        normalizedInput.inspectionTypeId = input.inspectionTypeId;
    }
    if (input.executionUnitId.has_value()) {
        normalizedInput.executionUnitId = input.executionUnitId;
    }
    if (input.toolingId.has_value()) {
        normalizedInput.toolingId = input.toolingId;
    }
    if (input.value.has_value()) {
        normalizedInput.value = normalizeOptionalText(input.value);
    }
    if (input.status.has_value()) {
        normalizedInput.status = normalizeOptionalText(input.status);
    }
    if (input.remarks.has_value()) {
        normalizedInput.remarks = normalizeOptionalText(input.remarks);
    }
    if (input.configSnapshot.has_value()) {
        normalizedInput.configSnapshot = normalizeOptionalText(input.configSnapshot);
    }

    return resultRepository_.updateById(resultId, normalizedInput);
}

bool ResultService::deleteResult(qint64 resultId) const {
    return resultRepository_.removeById(resultId);
}

std::optional<travis::models::ResultImage> ResultService::createResultImage(
    const travis::models::ResultImageCreateInput& input
) const {
    return resultImageRepository_.create({
        .resultId = input.resultId,
        .rawUrl = normalizeRequiredText(input.rawUrl, "Result image rawUrl is required"),
        .annotatedUrl = normalizeOptionalText(input.annotatedUrl),
        .remarks = normalizeOptionalText(input.remarks),
    });
}

QVector<travis::models::ResultImage> ResultService::listResultImages() const {
    return resultImageRepository_.listAll();
}

QVector<travis::models::ResultImage> ResultService::listResultImagesByResultId(qint64 resultId) const {
    return resultImageRepository_.listByResultId(resultId);
}

QVector<ResultImageSummary> ResultService::listResultImageSummariesByResultId(qint64 resultId) const {
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

std::optional<travis::models::ResultImage> ResultService::getResultImageById(qint64 imageId) const {
    return resultImageRepository_.findById(imageId);
}

std::optional<travis::models::ResultImage> ResultService::updateResultImage(
    qint64 imageId,
    const travis::models::ResultImageUpdateInput& input
) const {
    travis::models::ResultImageUpdateInput normalizedInput;

    if (input.resultId.has_value()) {
        normalizedInput.resultId = input.resultId;
    }
    if (input.rawUrl.has_value()) {
        normalizedInput.rawUrl = normalizeRequiredText(*input.rawUrl, "Result image rawUrl is required");
    }
    if (input.annotatedUrl.has_value()) {
        normalizedInput.annotatedUrl = normalizeOptionalText(input.annotatedUrl);
    }
    if (input.remarks.has_value()) {
        normalizedInput.remarks = normalizeOptionalText(input.remarks);
    }

    return resultImageRepository_.updateById(imageId, normalizedInput);
}

bool ResultService::deleteResultImage(qint64 imageId) const {
    return resultImageRepository_.removeById(imageId);
}

} // namespace travis::services
