#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/asset.h"

// Repository for CRUD access to the asset table.

namespace travis::data::repositories {

class AssetRepository : public BaseRepository {
public:
    explicit AssetRepository(QSqlDatabase database);

    std::optional<travis::models::Asset> create(const travis::models::AssetCreateInput& input) const;
    QVector<travis::models::Asset> listAll() const;
    QVector<travis::models::Asset> listByProjectId(qint64 projectId) const;
    std::optional<travis::models::Asset> findById(qint64 assetId) const;
    std::optional<travis::models::Asset> updateById(
        qint64 assetId,
        const travis::models::AssetUpdateInput& input
    ) const;
    bool removeById(qint64 assetId) const;
};

} // namespace travis::data::repositories
