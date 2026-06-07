#include "data/repositories/item_repository.h"

#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::Item;
using travis::models::ItemCreateInput;
using travis::models::ItemUpdateInput;

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

Item mapItem(const QSqlQuery& query) {
    return Item{
        .itemId = query.value("item_id").toLongLong(),
        .componentId = query.value("component_id").toLongLong(),
        .itemLabel = query.value("item_label").toString(),
        .position = optionalString(query, "position"),
        .status = optionalInteger(query, "status"),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

QVariant toNullableInteger(const std::optional<qint64>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::LongLong);
}

} // namespace

ItemRepository::ItemRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Item> ItemRepository::create(const ItemCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO item (component_id, item_label, position, status)
        VALUES (:component_id, :item_label, :position, :status)
    )");
    query.bindValue(":component_id", input.componentId);
    query.bindValue(":item_label", input.itemLabel);
    query.bindValue(":position", toNullableString(input.position));
    query.bindValue(":status", toNullableInteger(input.status));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Item> ItemRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT item_id, component_id, item_label, position, status
        FROM item
        ORDER BY component_id ASC, item_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Item> items;
    while (query.next()) {
        items.append(mapItem(query));
    }

    return items;
}

QVector<Item> ItemRepository::listByComponentId(qint64 componentId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT item_id, component_id, item_label, position, status
        FROM item
        WHERE component_id = :component_id
        ORDER BY item_id ASC
    )");
    query.bindValue(":component_id", componentId);

    if (!query.exec()) {
        return {};
    }

    QVector<Item> items;
    while (query.next()) {
        items.append(mapItem(query));
    }

    return items;
}

std::optional<Item> ItemRepository::findById(qint64 itemId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT item_id, component_id, item_label, position, status
        FROM item
        WHERE item_id = :item_id
    )");
    query.bindValue(":item_id", itemId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapItem(query);
}

std::optional<Item> ItemRepository::findByItemLabel(const QString& itemLabel) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT item_id, component_id, item_label, position, status
        FROM item
        WHERE item_label = :item_label
    )");
    query.bindValue(":item_label", itemLabel);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapItem(query);
}

std::optional<Item> ItemRepository::updateById(qint64 itemId, const ItemUpdateInput& input) const {
    const std::optional<Item> existingItem = findById(itemId);
    if (!existingItem.has_value()) {
        return std::nullopt;
    }

    const Item item = *existingItem;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE item
        SET
            component_id = :component_id,
            item_label = :item_label,
            position = :position,
            status = :status
        WHERE item_id = :item_id
    )");
    query.bindValue(":component_id", input.componentId.value_or(item.componentId));
    query.bindValue(":item_label", input.itemLabel.value_or(item.itemLabel));
    query.bindValue(":position", input.position ? toNullableString(input.position) : toNullableString(item.position));
    query.bindValue(":status", input.status ? toNullableInteger(input.status) : toNullableInteger(item.status));
    query.bindValue(":item_id", itemId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(itemId);
}

bool ItemRepository::removeById(qint64 itemId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM item WHERE item_id = :item_id");
    query.bindValue(":item_id", itemId);

    return query.exec();
}

} // namespace travis::data::repositories
