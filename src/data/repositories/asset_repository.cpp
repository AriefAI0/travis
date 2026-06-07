#include "data/repositories/asset_repository.h"

#include <QSqlQuery>

namespace travis::data::repositories {

namespace {

using travis::models::Asset;
using travis::models::AssetCreateInput;
using travis::models::AssetUpdateInput;

Asset mapAsset(const QSqlQuery& query) {
    return Asset{
        .assetId = query.value("asset_id").toLongLong(),
        .projectId = query.value("project_id").toLongLong(),
        .name = query.value("name").toString(),
        .createdAt = query.value("created_at").toLongLong(),
    };
}

} // namespace

AssetRepository::AssetRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Asset> AssetRepository::create(const AssetCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO asset (project_id, name)
        VALUES (:project_id, :name)
    )");
    query.bindValue(":project_id", input.projectId);
    query.bindValue(":name", input.name);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Asset> AssetRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT asset_id, project_id, name, created_at
        FROM asset
        ORDER BY project_id ASC, asset_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Asset> assets;
    while (query.next()) {
        assets.append(mapAsset(query));
    }

    return assets;
}

QVector<Asset> AssetRepository::listByProjectId(qint64 projectId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT asset_id, project_id, name, created_at
        FROM asset
        WHERE project_id = :project_id
        ORDER BY asset_id ASC
    )");
    query.bindValue(":project_id", projectId);

    if (!query.exec()) {
        return {};
    }

    QVector<Asset> assets;
    while (query.next()) {
        assets.append(mapAsset(query));
    }

    return assets;
}

std::optional<Asset> AssetRepository::findById(qint64 assetId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT asset_id, project_id, name, created_at
        FROM asset
        WHERE asset_id = :asset_id
    )");
    query.bindValue(":asset_id", assetId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapAsset(query);
}

std::optional<Asset> AssetRepository::updateById(qint64 assetId, const AssetUpdateInput& input) const {
    const std::optional<Asset> existingAsset = findById(assetId);
    if (!existingAsset.has_value()) {
        return std::nullopt;
    }

    const Asset asset = *existingAsset;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE asset
        SET
            project_id = :project_id,
            name = :name
        WHERE asset_id = :asset_id
    )");
    query.bindValue(":project_id", input.projectId.value_or(asset.projectId));
    query.bindValue(":name", input.name.value_or(asset.name));
    query.bindValue(":asset_id", assetId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(assetId);
}

bool AssetRepository::removeById(qint64 assetId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM asset WHERE asset_id = :asset_id");
    query.bindValue(":asset_id", assetId);

    return query.exec();
}

} // namespace travis::data::repositories
