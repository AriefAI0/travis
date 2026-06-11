#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QModelIndex>
#include <QVariantMap>
#include <QVector>

#include <memory>
#include <vector>

#include "services/structure_service.h"

// QML-facing tree model for project asset/component/item hierarchy.

namespace travis::ui::viewmodels {

class StructureTreeModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Role {
        NodeLabelRole = Qt::UserRole + 1,
        TypeRole,
        KeyRole,
        AssetRole,
        ComponentRole,
        ItemRole,
        DepthRole,
    };
    Q_ENUM(Role)

    explicit StructureTreeModel(QObject* parent = nullptr);
    ~StructureTreeModel() override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setStructure(const QVector<travis::services::StructureAssetNode>& assets);
    void clear();

private:
    struct TreeNode {
        QString type;
        QString key;
        QString display;
        QVariantMap asset;
        QVariantMap component;
        QVariantMap item;
        TreeNode* parent = nullptr;
        std::vector<std::unique_ptr<TreeNode>> children;
    };

    [[nodiscard]] TreeNode* nodeFromIndex(const QModelIndex& index) const;
    [[nodiscard]] int nodeRow(const TreeNode* node) const;

    std::unique_ptr<TreeNode> rootNode_;
};

} // namespace travis::ui::viewmodels
