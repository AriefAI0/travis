#include "data/repositories/execution_unit_repository.h"

#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::ExecutionUnit;
using travis::models::ExecutionUnitCreateInput;
using travis::models::ExecutionUnitUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

ExecutionUnit mapExecutionUnit(const QSqlQuery& query) {
    return ExecutionUnit{
        .executionUnitId = query.value("execution_unit_id").toLongLong(),
        .type = query.value("type").toString(),
        .name = query.value("name").toString(),
        .meta = optionalString(query, "meta"),
        .createdAt = query.value("created_at").toLongLong(),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

} // namespace

ExecutionUnitRepository::ExecutionUnitRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<ExecutionUnit> ExecutionUnitRepository::create(const ExecutionUnitCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO execution_unit (type, name, meta)
        VALUES (:type, :name, :meta)
    )");
    query.bindValue(":type", input.type);
    query.bindValue(":name", input.name);
    query.bindValue(":meta", toNullableString(input.meta));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<ExecutionUnit> ExecutionUnitRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT execution_unit_id, type, name, meta, created_at
        FROM execution_unit
        ORDER BY type ASC, name ASC, execution_unit_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<ExecutionUnit> executionUnits;
    while (query.next()) {
        executionUnits.append(mapExecutionUnit(query));
    }

    return executionUnits;
}

QVector<ExecutionUnit> ExecutionUnitRepository::listByType(const QString& type) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT execution_unit_id, type, name, meta, created_at
        FROM execution_unit
        WHERE type = :type
        ORDER BY name ASC, execution_unit_id ASC
    )");
    query.bindValue(":type", type);

    if (!query.exec()) {
        return {};
    }

    QVector<ExecutionUnit> executionUnits;
    while (query.next()) {
        executionUnits.append(mapExecutionUnit(query));
    }

    return executionUnits;
}

std::optional<ExecutionUnit> ExecutionUnitRepository::findById(qint64 executionUnitId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT execution_unit_id, type, name, meta, created_at
        FROM execution_unit
        WHERE execution_unit_id = :execution_unit_id
    )");
    query.bindValue(":execution_unit_id", executionUnitId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapExecutionUnit(query);
}

std::optional<ExecutionUnit> ExecutionUnitRepository::findByTypeAndName(const QString& type, const QString& name) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT execution_unit_id, type, name, meta, created_at
        FROM execution_unit
        WHERE type = :type AND name = :name
    )");
    query.bindValue(":type", type);
    query.bindValue(":name", name);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapExecutionUnit(query);
}

std::optional<ExecutionUnit> ExecutionUnitRepository::updateById(
    qint64 executionUnitId,
    const ExecutionUnitUpdateInput& input
) const {
    const std::optional<ExecutionUnit> existingExecutionUnit = findById(executionUnitId);
    if (!existingExecutionUnit.has_value()) {
        return std::nullopt;
    }

    const ExecutionUnit executionUnit = *existingExecutionUnit;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE execution_unit
        SET
            type = :type,
            name = :name,
            meta = :meta
        WHERE execution_unit_id = :execution_unit_id
    )");
    query.bindValue(":type", input.type.value_or(executionUnit.type));
    query.bindValue(":name", input.name.value_or(executionUnit.name));
    query.bindValue(":meta", input.meta ? toNullableString(input.meta) : toNullableString(executionUnit.meta));
    query.bindValue(":execution_unit_id", executionUnitId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(executionUnitId);
}

bool ExecutionUnitRepository::removeById(qint64 executionUnitId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM execution_unit WHERE execution_unit_id = :execution_unit_id");
    query.bindValue(":execution_unit_id", executionUnitId);

    return query.exec();
}

} // namespace travis::data::repositories
