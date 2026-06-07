#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/component.h"

// Repository for CRUD access to the component table.

namespace travis::data::repositories {

class ComponentRepository : public BaseRepository {
public:
    explicit ComponentRepository(QSqlDatabase database);

    std::optional<travis::models::Component> create(const travis::models::ComponentCreateInput& input) const;
    QVector<travis::models::Component> listAll() const;
    QVector<travis::models::Component> listByAssetId(qint64 assetId) const;
    std::optional<travis::models::Component> findById(qint64 componentId) const;
    std::optional<travis::models::Component> updateById(
        qint64 componentId,
        const travis::models::ComponentUpdateInput& input
    ) const;
    bool removeById(qint64 componentId) const;
};

} // namespace travis::data::repositories
