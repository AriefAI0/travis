#include "ui/viewmodels/structure_tree_model.h"

#include <QStringLiteral>

// Builds an efficient Qt tree model from the native structure service snapshot.

namespace travis::ui::viewmodels {

namespace {

QVariantMap toItemVariant(const travis::services::StructureItemNode& item) {
    return QVariantMap{
        {QStringLiteral("itemId"), item.itemId},
        {QStringLiteral("componentId"), item.componentId},
        {QStringLiteral("itemLabel"), item.itemLabel},
        {QStringLiteral("position"), item.position.value_or(QString{})},
        {QStringLiteral("status"), item.status.has_value() ? QVariant(*item.status) : QVariant{}},
    };
}

QVariantMap toComponentVariant(const travis::services::StructureComponentNode& component) {
    QVariantList items;
    items.reserve(component.items.size());

    for (const auto& item : component.items) {
        items.append(toItemVariant(item));
    }

    return QVariantMap{
        {QStringLiteral("componentId"), component.componentId},
        {QStringLiteral("assetId"), component.assetId},
        {QStringLiteral("name"), component.name},
        {QStringLiteral("items"), items},
    };
}

QVariantMap toAssetVariant(const travis::services::StructureAssetNode& asset) {
    QVariantList components;
    components.reserve(asset.components.size());

    for (const auto& component : asset.components) {
        components.append(toComponentVariant(component));
    }

    return QVariantMap{
        {QStringLiteral("assetId"), asset.assetId},
        {QStringLiteral("projectId"), asset.projectId},
        {QStringLiteral("name"), asset.name},
        {QStringLiteral("components"), components},
    };
}

} // namespace

StructureTreeModel::StructureTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootNode_(std::make_unique<TreeNode>()) {}

StructureTreeModel::~StructureTreeModel() = default;

QModelIndex StructureTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    const TreeNode* parentNode = nodeFromIndex(parent);
    if (!parentNode || row < 0 || row >= parentNode->children.size()) {
        return {};
    }

    return createIndex(row, column, parentNode->children.at(row).get());
}

QModelIndex StructureTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    const TreeNode* childNode = nodeFromIndex(child);
    if (!childNode || !childNode->parent || childNode->parent == rootNode_.get()) {
        return {};
    }

    return createIndex(nodeRow(childNode->parent), 0, childNode->parent);
}

int StructureTreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    const TreeNode* parentNode = nodeFromIndex(parent);
    return parentNode ? parentNode->children.size() : 0;
}

int StructureTreeModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 1;
}

QVariant StructureTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    const TreeNode* node = nodeFromIndex(index);
    if (!node) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
    case NodeLabelRole:
        return node->display;
    case TypeRole:
        return node->type;
    case KeyRole:
        return node->key;
    case AssetRole:
        return node->asset;
    case ComponentRole:
        return node->component;
    case ItemRole:
        return node->item;
    case DepthRole:
        return node->type == QStringLiteral("asset") ? 0 : node->type == QStringLiteral("component") ? 1 : 2;
    default:
        return {};
    }
}

QHash<int, QByteArray> StructureTreeModel::roleNames() const {
    return {
        {NodeLabelRole, "nodeLabel"},
        {TypeRole, "type"},
        {KeyRole, "key"},
        {AssetRole, "asset"},
        {ComponentRole, "component"},
        {ItemRole, "item"},
        {DepthRole, "depth"},
    };
}

void StructureTreeModel::setStructure(const QVector<travis::services::StructureAssetNode>& assets) {
    beginResetModel();
    rootNode_ = std::make_unique<TreeNode>();

    for (const auto& asset : assets) {
        auto assetNode = std::make_unique<TreeNode>();
        assetNode->type = QStringLiteral("asset");
        assetNode->key = QStringLiteral("asset:%1").arg(asset.assetId);
        assetNode->display = asset.name;
        assetNode->asset = toAssetVariant(asset);
        assetNode->parent = rootNode_.get();

        for (const auto& component : asset.components) {
            auto componentNode = std::make_unique<TreeNode>();
            componentNode->type = QStringLiteral("component");
            componentNode->key = QStringLiteral("component:%1").arg(component.componentId);
            componentNode->display = component.name;
            componentNode->asset = assetNode->asset;
            componentNode->component = toComponentVariant(component);
            componentNode->parent = assetNode.get();

            for (const auto& item : component.items) {
                auto itemNode = std::make_unique<TreeNode>();
                itemNode->type = QStringLiteral("item");
                itemNode->key = QStringLiteral("item:%1").arg(item.itemId);
                itemNode->display = item.itemLabel;
                itemNode->asset = assetNode->asset;
                itemNode->component = componentNode->component;
                itemNode->item = toItemVariant(item);
                itemNode->parent = componentNode.get();
                componentNode->children.push_back(std::move(itemNode));
            }

            assetNode->children.push_back(std::move(componentNode));
        }

        rootNode_->children.push_back(std::move(assetNode));
    }

    endResetModel();
}

void StructureTreeModel::clear() {
    beginResetModel();
    rootNode_ = std::make_unique<TreeNode>();
    endResetModel();
}

StructureTreeModel::TreeNode* StructureTreeModel::nodeFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return rootNode_.get();
    }

    return static_cast<TreeNode*>(index.internalPointer());
}

int StructureTreeModel::nodeRow(const TreeNode* node) const {
    if (!node || !node->parent) {
        return 0;
    }

    for (int row = 0; row < node->parent->children.size(); ++row) {
        if (node->parent->children.at(row).get() == node) {
            return row;
        }
    }

    return 0;
}

} // namespace travis::ui::viewmodels
