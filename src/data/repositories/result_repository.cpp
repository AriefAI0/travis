#include "data/repositories/result_repository.h"

#include <QDateTime>
#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::Result;
using travis::models::ResultCreateInput;
using travis::models::ResultUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

std::optional<qint64> optionalInteger(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toLongLong();
}

Result mapResult(const QSqlQuery& query) {
    return Result{
        .resultId = query.value("result_id").toLongLong(),
        .sessionItemId = query.value("session_item_id").toLongLong(),
        .inspectionTypeId = query.value("inspection_type_id").toLongLong(),
        .executionUnitId = query.value("execution_unit_id").toLongLong(),
        .toolingId = optionalInteger(query, "tooling_id"),
        .value = optionalString(query, "value"),
        .status = optionalString(query, "status"),
        .remarks = optionalString(query, "remarks"),
        .configSnapshot = optionalString(query, "config_snapshot"),
        .createdAt = query.value("created_at").toLongLong(),
        .updatedAt = query.value("updated_at").toLongLong(),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

QVariant toNullableInteger(const std::optional<qint64>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::LongLong);
}

qint64 currentEpochSeconds() {
    return QDateTime::currentSecsSinceEpoch();
}

} // namespace

ResultRepository::ResultRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Result> ResultRepository::create(const ResultCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO result (
            session_item_id,
            inspection_type_id,
            execution_unit_id,
            tooling_id,
            value,
            status,
            remarks,
            config_snapshot
        )
        VALUES (
            :session_item_id,
            :inspection_type_id,
            :execution_unit_id,
            :tooling_id,
            :value,
            :status,
            :remarks,
            :config_snapshot
        )
    )");
    query.bindValue(":session_item_id", input.sessionItemId);
    query.bindValue(":inspection_type_id", input.inspectionTypeId);
    query.bindValue(":execution_unit_id", input.executionUnitId);
    query.bindValue(":tooling_id", toNullableInteger(input.toolingId));
    query.bindValue(":value", toNullableString(input.value));
    query.bindValue(":status", toNullableString(input.status));
    query.bindValue(":remarks", toNullableString(input.remarks));
    query.bindValue(":config_snapshot", toNullableString(input.configSnapshot));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Result> ResultRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            result_id,
            session_item_id,
            inspection_type_id,
            execution_unit_id,
            tooling_id,
            value,
            status,
            remarks,
            config_snapshot,
            created_at,
            updated_at
        FROM result
        ORDER BY session_item_id ASC, result_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Result> results;
    while (query.next()) {
        results.append(mapResult(query));
    }

    return results;
}

QVector<Result> ResultRepository::listBySessionItemId(qint64 sessionItemId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            result_id,
            session_item_id,
            inspection_type_id,
            execution_unit_id,
            tooling_id,
            value,
            status,
            remarks,
            config_snapshot,
            created_at,
            updated_at
        FROM result
        WHERE session_item_id = :session_item_id
        ORDER BY result_id ASC
    )");
    query.bindValue(":session_item_id", sessionItemId);

    if (!query.exec()) {
        return {};
    }

    QVector<Result> results;
    while (query.next()) {
        results.append(mapResult(query));
    }

    return results;
}

std::optional<Result> ResultRepository::findById(qint64 resultId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            result_id,
            session_item_id,
            inspection_type_id,
            execution_unit_id,
            tooling_id,
            value,
            status,
            remarks,
            config_snapshot,
            created_at,
            updated_at
        FROM result
        WHERE result_id = :result_id
    )");
    query.bindValue(":result_id", resultId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapResult(query);
}

std::optional<Result> ResultRepository::findBySessionItemInspectionTypeAndStatus(
    qint64 sessionItemId,
    qint64 inspectionTypeId,
    const QString& status
) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            result_id,
            session_item_id,
            inspection_type_id,
            execution_unit_id,
            tooling_id,
            value,
            status,
            remarks,
            config_snapshot,
            created_at,
            updated_at
        FROM result
        WHERE
            session_item_id = :session_item_id AND
            inspection_type_id = :inspection_type_id AND
            status = :status
        ORDER BY result_id ASC
        LIMIT 1
    )");
    query.bindValue(":session_item_id", sessionItemId);
    query.bindValue(":inspection_type_id", inspectionTypeId);
    query.bindValue(":status", status);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapResult(query);
}

std::optional<Result> ResultRepository::updateById(qint64 resultId, const ResultUpdateInput& input) const {
    const std::optional<Result> existingResult = findById(resultId);
    if (!existingResult.has_value()) {
        return std::nullopt;
    }

    const Result result = *existingResult;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE result
        SET
            session_item_id = :session_item_id,
            inspection_type_id = :inspection_type_id,
            execution_unit_id = :execution_unit_id,
            tooling_id = :tooling_id,
            value = :value,
            status = :status,
            remarks = :remarks,
            config_snapshot = :config_snapshot,
            updated_at = :updated_at
        WHERE result_id = :result_id
    )");
    query.bindValue(":session_item_id", input.sessionItemId.value_or(result.sessionItemId));
    query.bindValue(":inspection_type_id", input.inspectionTypeId.value_or(result.inspectionTypeId));
    query.bindValue(":execution_unit_id", input.executionUnitId.value_or(result.executionUnitId));
    query.bindValue(":tooling_id", input.toolingId ? toNullableInteger(input.toolingId) : toNullableInteger(result.toolingId));
    query.bindValue(":value", input.value ? toNullableString(input.value) : toNullableString(result.value));
    query.bindValue(":status", input.status ? toNullableString(input.status) : toNullableString(result.status));
    query.bindValue(":remarks", input.remarks ? toNullableString(input.remarks) : toNullableString(result.remarks));
    query.bindValue(":config_snapshot", input.configSnapshot ? toNullableString(input.configSnapshot) : toNullableString(result.configSnapshot));
    query.bindValue(":updated_at", currentEpochSeconds());
    query.bindValue(":result_id", resultId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(resultId);
}

bool ResultRepository::removeById(qint64 resultId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM result WHERE result_id = :result_id");
    query.bindValue(":result_id", resultId);

    return query.exec();
}

} // namespace travis::data::repositories
