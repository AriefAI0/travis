#include "data/repositories/inspection_type_repository.h"

#include <QSqlQuery>

namespace travis::data::repositories {

namespace {

using travis::models::InspectionType;
using travis::models::InspectionTypeCreateInput;
using travis::models::InspectionTypeUpdateInput;

InspectionType mapInspectionType(const QSqlQuery& query) {
    return InspectionType{
        .inspectionTypeId = query.value("inspection_type_id").toLongLong(),
        .name = query.value("name").toString(),
    };
}

} // namespace

InspectionTypeRepository::InspectionTypeRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<InspectionType> InspectionTypeRepository::create(const InspectionTypeCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO inspection_type (name)
        VALUES (:name)
    )");
    query.bindValue(":name", input.name);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

std::optional<InspectionType> InspectionTypeRepository::createIfMissing(const InspectionTypeCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT OR IGNORE INTO inspection_type (name)
        VALUES (:name)
    )");
    query.bindValue(":name", input.name);

    if (!query.exec()) {
        return std::nullopt;
    }

    if (query.numRowsAffected() > 0) {
        return findById(query.lastInsertId().toLongLong());
    }

    return findByName(input.name);
}

QVector<InspectionType> InspectionTypeRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT inspection_type_id, name
        FROM inspection_type
        ORDER BY name ASC, inspection_type_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<InspectionType> inspectionTypes;
    while (query.next()) {
        inspectionTypes.append(mapInspectionType(query));
    }

    return inspectionTypes;
}

std::optional<InspectionType> InspectionTypeRepository::findById(qint64 inspectionTypeId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT inspection_type_id, name
        FROM inspection_type
        WHERE inspection_type_id = :inspection_type_id
    )");
    query.bindValue(":inspection_type_id", inspectionTypeId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapInspectionType(query);
}

std::optional<InspectionType> InspectionTypeRepository::findByName(const QString& name) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT inspection_type_id, name
        FROM inspection_type
        WHERE name = :name
    )");
    query.bindValue(":name", name);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapInspectionType(query);
}

std::optional<InspectionType> InspectionTypeRepository::updateById(
    qint64 inspectionTypeId,
    const InspectionTypeUpdateInput& input
) const {
    const std::optional<InspectionType> existingInspectionType = findById(inspectionTypeId);
    if (!existingInspectionType.has_value()) {
        return std::nullopt;
    }

    const InspectionType inspectionType = *existingInspectionType;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE inspection_type
        SET
            name = :name
        WHERE inspection_type_id = :inspection_type_id
    )");
    query.bindValue(":name", input.name.value_or(inspectionType.name));
    query.bindValue(":inspection_type_id", inspectionTypeId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(inspectionTypeId);
}

bool InspectionTypeRepository::removeById(qint64 inspectionTypeId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM inspection_type WHERE inspection_type_id = :inspection_type_id");
    query.bindValue(":inspection_type_id", inspectionTypeId);

    return query.exec();
}

} // namespace travis::data::repositories
