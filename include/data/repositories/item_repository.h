#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/item.h"

// Repository for CRUD access to the item table.

namespace travis::data::repositories {

class ItemRepository : public BaseRepository {
public:
    explicit ItemRepository(QSqlDatabase database);

    std::optional<travis::models::Item> create(const travis::models::ItemCreateInput& input) const;
    QVector<travis::models::Item> listAll() const;
    QVector<travis::models::Item> listByComponentId(qint64 componentId) const;
    std::optional<travis::models::Item> findById(qint64 itemId) const;
    std::optional<travis::models::Item> findByItemLabel(const QString& itemLabel) const;
    std::optional<travis::models::Item> updateById(
        qint64 itemId,
        const travis::models::ItemUpdateInput& input
    ) const;
    bool removeById(qint64 itemId) const;
};

} // namespace travis::data::repositories
