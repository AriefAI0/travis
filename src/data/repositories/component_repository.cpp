#include "data/repositories/component_repository.h"

#include <QSqlQuery>

namespace travis::data::repositories {

namespace {

using travis::models::Component;
using travis::models::ComponentCreateInput;
using travis::models::ComponentUpdateInput;

Component mapComponent(const QSqlQuery& query) {
    return Component{
        .componentId = query.value("component_id").toLongLong(),
        .assetId = query.value("asset_id").toLongLong(),
        .name = query.value("name").toString(),
    };
}

} // namespace

ComponentRepository::ComponentRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Component> ComponentRepository::create(const ComponentCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO component (asset_id, name)
        VALUES (:asset_id, :name)
    )");
    query.bindValue(":asset_id", input.assetId);
    query.bindValue(":name", input.name);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Component> ComponentRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT component_id, asset_id, name
        FROM component
        ORDER BY asset_id ASC, component_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Component> components;
    while (query.next()) {
        components.append(mapComponent(query));
    }

    return components;
}

QVector<Component> ComponentRepository::listByAssetId(qint64 assetId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT component_id, asset_id, name
        FROM component
        WHERE asset_id = :asset_id
        ORDER BY component_id ASC
    )");
    query.bindValue(":asset_id", assetId);

    if (!query.exec()) {
        return {};
    }

    QVector<Component> components;
    while (query.next()) {
        components.append(mapComponent(query));
    }

    return components;
}

std::optional<Component> ComponentRepository::findById(qint64 componentId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT component_id, asset_id, name
        FROM component
        WHERE component_id = :component_id
    )");
    query.bindValue(":component_id", componentId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapComponent(query);
}

std::optional<Component> ComponentRepository::updateById(qint64 componentId, const ComponentUpdateInput& input) const {
    const std::optional<Component> existingComponent = findById(componentId);
    if (!existingComponent.has_value()) {
        return std::nullopt;
    }

    const Component component = *existingComponent;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE component
        SET
            asset_id = :asset_id,
            name = :name
        WHERE component_id = :component_id
    )");
    query.bindValue(":asset_id", input.assetId.value_or(component.assetId));
    query.bindValue(":name", input.name.value_or(component.name));
    query.bindValue(":component_id", componentId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(componentId);
}

bool ComponentRepository::removeById(qint64 componentId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM component WHERE component_id = :component_id");
    query.bindValue(":component_id", componentId);

    return query.exec();
}

} // namespace travis::data::repositories
