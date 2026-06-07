#pragma once

#include <QString>
#include <QVector>

#include <optional>

#include "data/repositories/asset_repository.h"
#include "data/repositories/component_repository.h"
#include "data/repositories/item_repository.h"
#include "models/asset.h"
#include "models/component.h"
#include "models/item.h"

// Service layer for asset/component/item validation and structure traversal.

namespace travis::services {

struct StructureItemNode {
    qint64 itemId = 0;
    qint64 componentId = 0;
    QString itemLabel;
    std::optional<QString> position;
    std::optional<qint64> status;
};

struct StructureComponentNode {
    qint64 componentId = 0;
    qint64 assetId = 0;
    QString name;
    QVector<StructureItemNode> items;
};

struct StructureAssetNode {
    qint64 assetId = 0;
    qint64 projectId = 0;
    QString name;
    QVector<StructureComponentNode> components;
};

class StructureService {
public:
    StructureService(
        travis::data::repositories::AssetRepository assetRepository,
        travis::data::repositories::ComponentRepository componentRepository,
        travis::data::repositories::ItemRepository itemRepository
    );

    std::optional<travis::models::Asset> createAsset(const travis::models::AssetCreateInput& input) const;
    QVector<travis::models::Asset> listAssets() const;
    QVector<travis::models::Asset> listAssetsByProjectId(qint64 projectId) const;
    std::optional<travis::models::Asset> getAssetById(qint64 assetId) const;
    std::optional<travis::models::Asset> updateAsset(
        qint64 assetId,
        const travis::models::AssetUpdateInput& input
    ) const;
    bool deleteAsset(qint64 assetId) const;

    std::optional<travis::models::Component> createComponent(
        const travis::models::ComponentCreateInput& input
    ) const;
    QVector<travis::models::Component> listComponents() const;
    QVector<travis::models::Component> listComponentsByAssetId(qint64 assetId) const;
    std::optional<travis::models::Component> getComponentById(qint64 componentId) const;
    std::optional<travis::models::Component> updateComponent(
        qint64 componentId,
        const travis::models::ComponentUpdateInput& input
    ) const;
    bool deleteComponent(qint64 componentId) const;

    std::optional<travis::models::Item> createItem(const travis::models::ItemCreateInput& input) const;
    QVector<travis::models::Item> listItems() const;
    QVector<travis::models::Item> listItemsByComponentId(qint64 componentId) const;
    std::optional<travis::models::Item> getItemById(qint64 itemId) const;
    std::optional<travis::models::Item> getItemByLabel(const QString& itemLabel) const;
    std::optional<travis::models::Item> updateItem(
        qint64 itemId,
        const travis::models::ItemUpdateInput& input
    ) const;
    bool deleteItem(qint64 itemId) const;

    QVector<StructureAssetNode> listProjectStructureTree(qint64 projectId) const;

private:
    travis::data::repositories::AssetRepository assetRepository_;
    travis::data::repositories::ComponentRepository componentRepository_;
    travis::data::repositories::ItemRepository itemRepository_;
};

} // namespace travis::services
