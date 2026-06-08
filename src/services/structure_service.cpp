#include "services/structure_service.h"

#include <stdexcept>

namespace travis::services {

namespace {

QString normalizeRequiredName(const QString& value, const char* fieldName) {
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        throw std::runtime_error(fieldName);
    }

    return trimmedValue;
}

std::optional<QString> normalizeOptionalText(const std::optional<QString>& value) {
    if (!value.has_value()) {
        return std::nullopt;
    }

    const QString trimmedValue = value->trimmed();
    if (trimmedValue.isEmpty()) {
        return std::nullopt;
    }

    return trimmedValue;
}

} // namespace

StructureService::StructureService(
    travis::data::repositories::AssetRepository assetRepository,
    travis::data::repositories::ComponentRepository componentRepository,
    travis::data::repositories::ItemRepository itemRepository
)
    : assetRepository_(std::move(assetRepository))
    , componentRepository_(std::move(componentRepository))
    , itemRepository_(std::move(itemRepository)) {}

std::optional<travis::models::Asset> StructureService::createAsset(
    const travis::models::AssetCreateInput& input
) const {
    return assetRepository_.create({
        .projectId = input.projectId,
        .name = normalizeRequiredName(input.name, "Asset name is required"),
    });
}

QVector<travis::models::Asset> StructureService::listAssets() const {
    return assetRepository_.listAll();
}

QVector<travis::models::Asset> StructureService::listAssetsByProjectId(qint64 projectId) const {
    return assetRepository_.listByProjectId(projectId);
}

std::optional<travis::models::Asset> StructureService::getAssetById(qint64 assetId) const {
    return assetRepository_.findById(assetId);
}

std::optional<travis::models::Asset> StructureService::updateAsset(
    qint64 assetId,
    const travis::models::AssetUpdateInput& input
) const {
    travis::models::AssetUpdateInput normalizedInput;

    if (input.projectId.has_value()) {
        normalizedInput.projectId = input.projectId;
    }

    if (input.name.has_value()) {
        normalizedInput.name = normalizeRequiredName(*input.name, "Asset name is required");
    }

    return assetRepository_.updateById(assetId, normalizedInput);
}

bool StructureService::deleteAsset(qint64 assetId) const {
    return assetRepository_.removeById(assetId);
}

std::optional<travis::models::Component> StructureService::createComponent(
    const travis::models::ComponentCreateInput& input
) const {
    return componentRepository_.create({
        .assetId = input.assetId,
        .name = normalizeRequiredName(input.name, "Component name is required"),
    });
}

QVector<travis::models::Component> StructureService::listComponents() const {
    return componentRepository_.listAll();
}

QVector<travis::models::Component> StructureService::listComponentsByAssetId(qint64 assetId) const {
    return componentRepository_.listByAssetId(assetId);
}

std::optional<travis::models::Component> StructureService::getComponentById(qint64 componentId) const {
    return componentRepository_.findById(componentId);
}

std::optional<travis::models::Component> StructureService::updateComponent(
    qint64 componentId,
    const travis::models::ComponentUpdateInput& input
) const {
    travis::models::ComponentUpdateInput normalizedInput;

    if (input.assetId.has_value()) {
        normalizedInput.assetId = input.assetId;
    }

    if (input.name.has_value()) {
        normalizedInput.name = normalizeRequiredName(*input.name, "Component name is required");
    }

    return componentRepository_.updateById(componentId, normalizedInput);
}

bool StructureService::deleteComponent(qint64 componentId) const {
    return componentRepository_.removeById(componentId);
}

std::optional<travis::models::Item> StructureService::createItem(
    const travis::models::ItemCreateInput& input
) const {
    return itemRepository_.create({
        .componentId = input.componentId,
        .itemLabel = normalizeRequiredName(input.itemLabel, "Item label is required"),
        .position = normalizeOptionalText(input.position),
        .status = input.status,
    });
}

QVector<travis::models::Item> StructureService::listItems() const {
    return itemRepository_.listAll();
}

QVector<travis::models::Item> StructureService::listItemsByComponentId(qint64 componentId) const {
    return itemRepository_.listByComponentId(componentId);
}

std::optional<travis::models::Item> StructureService::getItemById(qint64 itemId) const {
    return itemRepository_.findById(itemId);
}

std::optional<travis::models::Item> StructureService::getItemByLabel(const QString& itemLabel) const {
    return itemRepository_.findByItemLabel(normalizeRequiredName(itemLabel, "Item label is required"));
}

std::optional<travis::models::Item> StructureService::updateItem(
    qint64 itemId,
    const travis::models::ItemUpdateInput& input
) const {
    travis::models::ItemUpdateInput normalizedInput;

    if (input.componentId.has_value()) {
        normalizedInput.componentId = input.componentId;
    }

    if (input.itemLabel.has_value()) {
        normalizedInput.itemLabel = normalizeRequiredName(*input.itemLabel, "Item label is required");
    }

    if (input.position.has_value()) {
        normalizedInput.position = normalizeOptionalText(*input.position);
    }

    if (input.status.has_value()) {
        normalizedInput.status = *input.status;
    }

    return itemRepository_.updateById(itemId, normalizedInput);
}

bool StructureService::deleteItem(qint64 itemId) const {
    return itemRepository_.removeById(itemId);
}

QVector<StructureAssetNode> StructureService::listProjectStructureTree(qint64 projectId) const {
    const QVector<travis::models::Asset> assets = assetRepository_.listByProjectId(projectId);
    QVector<StructureAssetNode> tree;
    tree.reserve(assets.size());

    for (const travis::models::Asset& asset : assets) {
        const QVector<travis::models::Component> components = componentRepository_.listByAssetId(asset.assetId);
        QVector<StructureComponentNode> componentNodes;
        componentNodes.reserve(components.size());

        for (const travis::models::Component& component : components) {
            const QVector<travis::models::Item> items = itemRepository_.listByComponentId(component.componentId);
            QVector<StructureItemNode> itemNodes;
            itemNodes.reserve(items.size());

            for (const travis::models::Item& item : items) {
                itemNodes.append({
                    .itemId = item.itemId,
                    .componentId = item.componentId,
                    .itemLabel = item.itemLabel,
                    .position = item.position,
                    .status = item.status,
                });
            }

            componentNodes.append({
                .componentId = component.componentId,
                .assetId = component.assetId,
                .name = component.name,
                .items = itemNodes,
            });
        }

        tree.append({
            .assetId = asset.assetId,
            .projectId = asset.projectId,
            .name = asset.name,
            .components = componentNodes,
        });
    }

    return tree;
}

} // namespace travis::services
