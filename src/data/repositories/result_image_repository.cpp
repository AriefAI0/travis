#include "data/repositories/result_image_repository.h"

#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::ResultImage;
using travis::models::ResultImageCreateInput;
using travis::models::ResultImageUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

ResultImage mapResultImage(const QSqlQuery& query) {
    return ResultImage{
        .imageId = query.value("image_id").toLongLong(),
        .resultId = query.value("result_id").toLongLong(),
        .rawUrl = query.value("raw_url").toString(),
        .annotatedUrl = optionalString(query, "annotated_url"),
        .remarks = optionalString(query, "remarks"),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

} // namespace

ResultImageRepository::ResultImageRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<ResultImage> ResultImageRepository::create(const ResultImageCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO result_image (result_id, raw_url, annotated_url, remarks)
        VALUES (:result_id, :raw_url, :annotated_url, :remarks)
    )");
    query.bindValue(":result_id", input.resultId);
    query.bindValue(":raw_url", input.rawUrl);
    query.bindValue(":annotated_url", toNullableString(input.annotatedUrl));
    query.bindValue(":remarks", toNullableString(input.remarks));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<ResultImage> ResultImageRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT image_id, result_id, raw_url, annotated_url, remarks
        FROM result_image
        ORDER BY image_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<ResultImage> resultImages;
    while (query.next()) {
        resultImages.append(mapResultImage(query));
    }

    return resultImages;
}

QVector<ResultImage> ResultImageRepository::listByResultId(qint64 resultId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT image_id, result_id, raw_url, annotated_url, remarks
        FROM result_image
        WHERE result_id = :result_id
        ORDER BY image_id ASC
    )");
    query.bindValue(":result_id", resultId);

    if (!query.exec()) {
        return {};
    }

    QVector<ResultImage> resultImages;
    while (query.next()) {
        resultImages.append(mapResultImage(query));
    }

    return resultImages;
}

std::optional<ResultImage> ResultImageRepository::findById(qint64 imageId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT image_id, result_id, raw_url, annotated_url, remarks
        FROM result_image
        WHERE image_id = :image_id
    )");
    query.bindValue(":image_id", imageId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapResultImage(query);
}

std::optional<ResultImage> ResultImageRepository::updateById(
    qint64 imageId,
    const ResultImageUpdateInput& input
) const {
    const std::optional<ResultImage> existingResultImage = findById(imageId);
    if (!existingResultImage.has_value()) {
        return std::nullopt;
    }

    const ResultImage resultImage = *existingResultImage;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE result_image
        SET
            result_id = :result_id,
            raw_url = :raw_url,
            annotated_url = :annotated_url,
            remarks = :remarks
        WHERE image_id = :image_id
    )");
    query.bindValue(":result_id", input.resultId.value_or(resultImage.resultId));
    query.bindValue(":raw_url", input.rawUrl.value_or(resultImage.rawUrl));
    query.bindValue(":annotated_url", input.annotatedUrl ? toNullableString(input.annotatedUrl) : toNullableString(resultImage.annotatedUrl));
    query.bindValue(":remarks", input.remarks ? toNullableString(input.remarks) : toNullableString(resultImage.remarks));
    query.bindValue(":image_id", imageId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(imageId);
}

bool ResultImageRepository::removeById(qint64 imageId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM result_image WHERE image_id = :image_id");
    query.bindValue(":image_id", imageId);

    return query.exec();
}

} // namespace travis::data::repositories
