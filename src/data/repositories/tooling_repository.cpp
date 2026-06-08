#include "data/repositories/tooling_repository.h"

#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::Tooling;
using travis::models::ToolingCreateInput;
using travis::models::ToolingUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

Tooling mapTooling(const QSqlQuery& query) {
    return Tooling{
        .toolingId = query.value("tooling_id").toLongLong(),
        .executionUnitId = query.value("execution_unit_id").toLongLong(),
        .name = query.value("name").toString(),
        .config = optionalString(query, "config"),
        .createdAt = query.value("created_at").toLongLong(),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

} // namespace

ToolingRepository::ToolingRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Tooling> ToolingRepository::create(const ToolingCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO tooling (execution_unit_id, name, config)
        VALUES (:execution_unit_id, :name, :config)
    )");
    query.bindValue(":execution_unit_id", input.executionUnitId);
    query.bindValue(":name", input.name);
    query.bindValue(":config", toNullableString(input.config));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Tooling> ToolingRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT tooling_id, execution_unit_id, name, config, created_at
        FROM tooling
        ORDER BY execution_unit_id ASC, tooling_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Tooling> toolings;
    while (query.next()) {
        toolings.append(mapTooling(query));
    }

    return toolings;
}

QVector<Tooling> ToolingRepository::listByExecutionUnitId(qint64 executionUnitId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT tooling_id, execution_unit_id, name, config, created_at
        FROM tooling
        WHERE execution_unit_id = :execution_unit_id
        ORDER BY tooling_id ASC
    )");
    query.bindValue(":execution_unit_id", executionUnitId);

    if (!query.exec()) {
        return {};
    }

    QVector<Tooling> toolings;
    while (query.next()) {
        toolings.append(mapTooling(query));
    }

    return toolings;
}

std::optional<Tooling> ToolingRepository::findById(qint64 toolingId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT tooling_id, execution_unit_id, name, config, created_at
        FROM tooling
        WHERE tooling_id = :tooling_id
    )");
    query.bindValue(":tooling_id", toolingId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapTooling(query);
}

std::optional<Tooling> ToolingRepository::findByExecutionUnitIdAndName(qint64 executionUnitId, const QString& name) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT tooling_id, execution_unit_id, name, config, created_at
        FROM tooling
        WHERE execution_unit_id = :execution_unit_id AND name = :name
    )");
    query.bindValue(":execution_unit_id", executionUnitId);
    query.bindValue(":name", name);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapTooling(query);
}

std::optional<Tooling> ToolingRepository::updateById(qint64 toolingId, const ToolingUpdateInput& input) const {
    const std::optional<Tooling> existingTooling = findById(toolingId);
    if (!existingTooling.has_value()) {
        return std::nullopt;
    }

    const Tooling tooling = *existingTooling;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE tooling
        SET
            execution_unit_id = :execution_unit_id,
            name = :name,
            config = :config
        WHERE tooling_id = :tooling_id
    )");
    query.bindValue(":execution_unit_id", input.executionUnitId.value_or(tooling.executionUnitId));
    query.bindValue(":name", input.name.value_or(tooling.name));
    query.bindValue(
        ":config",
        input.config.has_value() ? toNullableString(*input.config) : toNullableString(tooling.config)
    );
    query.bindValue(":tooling_id", toolingId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(toolingId);
}

bool ToolingRepository::removeById(qint64 toolingId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM tooling WHERE tooling_id = :tooling_id");
    query.bindValue(":tooling_id", toolingId);

    return query.exec();
}

} // namespace travis::data::repositories
