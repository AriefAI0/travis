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

    // Creates an asset after validating the name.
    std::optional<travis::models::Asset> createAsset(const travis::models::AssetCreateInput& input) const;
    // Returns all assets.
    QVector<travis::models::Asset> listAssets() const;
    // Returns assets for one project.
    QVector<travis::models::Asset> listAssetsByProjectId(qint64 projectId) const;
    // Returns one asset by id.
    std::optional<travis::models::Asset> getAssetById(qint64 assetId) const;
    // Updates one asset after validating the name when provided.
    std::optional<travis::models::Asset> updateAsset(
        qint64 assetId,
        const travis::models::AssetUpdateInput& input
    ) const;
    // Deletes one asset by id.
    bool deleteAsset(qint64 assetId) const;

    // Creates a component after validating the name.
    std::optional<travis::models::Component> createComponent(
        const travis::models::ComponentCreateInput& input
    ) const;
    // Returns all components.
    QVector<travis::models::Component> listComponents() const;
    // Returns components for one asset.
    QVector<travis::models::Component> listComponentsByAssetId(qint64 assetId) const;
    // Returns one component by id.
    std::optional<travis::models::Component> getComponentById(qint64 componentId) const;
    // Updates one component after validating the name when provided.
    std::optional<travis::models::Component> updateComponent(
        qint64 componentId,
        const travis::models::ComponentUpdateInput& input
    ) const;
    // Deletes one component by id.
    bool deleteComponent(qint64 componentId) const;

    // Creates an item after validating the label.
    std::optional<travis::models::Item> createItem(const travis::models::ItemCreateInput& input) const;
    // Returns all items.
    QVector<travis::models::Item> listItems() const;
    // Returns items for one component.
    QVector<travis::models::Item> listItemsByComponentId(qint64 componentId) const;
    // Returns one item by id.
    std::optional<travis::models::Item> getItemById(qint64 itemId) const;
    // Returns one item by label.
    std::optional<travis::models::Item> getItemByLabel(const QString& itemLabel) const;
    // Updates one item after validating the label when provided.
    std::optional<travis::models::Item> updateItem(
        qint64 itemId,
        const travis::models::ItemUpdateInput& input
    ) const;
    // Deletes one item by id.
    bool deleteItem(qint64 itemId) const;

    // Builds the nested asset -> component -> item tree for one project.
    QVector<StructureAssetNode> listProjectStructureTree(qint64 projectId) const;

private:
    travis::data::repositories::AssetRepository assetRepository_;
    travis::data::repositories::ComponentRepository componentRepository_;
    travis::data::repositories::ItemRepository itemRepository_;
};

} // namespace travis::services
