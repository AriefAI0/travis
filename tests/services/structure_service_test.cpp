#include <QtTest>

#include "data/repositories/asset_repository.h"
#include "data/repositories/component_repository.h"
#include "data/repositories/item_repository.h"
#include "services/structure_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class StructureServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema used by structure service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies asset and component creation validate and normalize names.
    void createAssetAndComponent_persistNormalizedNames();
    // Verifies item creation and update normalize optional text fields.
    void itemLifecycle_normalizesFields();
    // Verifies the project structure tree returns the nested hierarchy.
    void listProjectStructureTree_returnsNestedNodes();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void StructureServiceTest::init() {
    connectionName_ = QStringLiteral("structure_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");

    QSqlQuery query(database_);
    QVERIFY(query.exec("INSERT INTO project (title) VALUES ('Project One')"));
}

void StructureServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void StructureServiceTest::createAssetAndComponent_persistNormalizedNames() {
    travis::data::repositories::AssetRepository assetRepository(database_);
    travis::data::repositories::ComponentRepository componentRepository(database_);
    travis::data::repositories::ItemRepository itemRepository(database_);
    travis::services::StructureService service(assetRepository, componentRepository, itemRepository);

    const auto createdAsset = service.createAsset({
        .projectId = 1,
        .name = QStringLiteral("  Asset A  "),
    });

    QVERIFY(createdAsset.has_value());
    QCOMPARE(createdAsset->name, QStringLiteral("Asset A"));

    const auto createdComponent = service.createComponent({
        .assetId = createdAsset->assetId,
        .name = QStringLiteral("  Component A  "),
    });

    QVERIFY(createdComponent.has_value());
    QCOMPARE(createdComponent->name, QStringLiteral("Component A"));
}

void StructureServiceTest::itemLifecycle_normalizesFields() {
    travis::data::repositories::AssetRepository assetRepository(database_);
    travis::data::repositories::ComponentRepository componentRepository(database_);
    travis::data::repositories::ItemRepository itemRepository(database_);
    travis::services::StructureService service(assetRepository, componentRepository, itemRepository);

    const auto createdAsset = service.createAsset({ .projectId = 1, .name = QStringLiteral("Asset B") });
    QVERIFY(createdAsset.has_value());

    const auto createdComponent = service.createComponent({
        .assetId = createdAsset->assetId,
        .name = QStringLiteral("Component B"),
    });
    QVERIFY(createdComponent.has_value());

    const auto createdItem = service.createItem({
        .componentId = createdComponent->componentId,
        .itemLabel = QStringLiteral("  ITEM-100  "),
        .position = QStringLiteral("  Port Side  "),
        .status = 3,
    });

    QVERIFY(createdItem.has_value());
    QCOMPARE(createdItem->itemLabel, QStringLiteral("ITEM-100"));
    QCOMPARE(createdItem->position.value(), QStringLiteral("Port Side"));

    const auto updatedItem = service.updateItem(
        createdItem->itemId,
        {
            .position = QStringLiteral("   "),
            .status = 4,
        }
    );

    QVERIFY(updatedItem.has_value());
    QVERIFY(!updatedItem->position.has_value());
    QCOMPARE(updatedItem->status.value(), 4);
}

void StructureServiceTest::listProjectStructureTree_returnsNestedNodes() {
    travis::data::repositories::AssetRepository assetRepository(database_);
    travis::data::repositories::ComponentRepository componentRepository(database_);
    travis::data::repositories::ItemRepository itemRepository(database_);
    travis::services::StructureService service(assetRepository, componentRepository, itemRepository);

    const auto createdAsset = service.createAsset({ .projectId = 1, .name = QStringLiteral("Asset Tree") });
    QVERIFY(createdAsset.has_value());

    const auto createdComponent = service.createComponent({
        .assetId = createdAsset->assetId,
        .name = QStringLiteral("Component Tree"),
    });
    QVERIFY(createdComponent.has_value());

    const auto createdItem = service.createItem({
        .componentId = createdComponent->componentId,
        .itemLabel = QStringLiteral("TREE-ITEM"),
        .position = QStringLiteral("Top"),
        .status = 1,
    });
    QVERIFY(createdItem.has_value());

    const QVector<travis::services::StructureAssetNode> tree = service.listProjectStructureTree(1);
    QCOMPARE(tree.size(), 1);
    QCOMPARE(tree.first().name, QStringLiteral("Asset Tree"));
    QCOMPARE(tree.first().components.size(), 1);
    QCOMPARE(tree.first().components.first().name, QStringLiteral("Component Tree"));
    QCOMPARE(tree.first().components.first().items.size(), 1);
    QCOMPARE(tree.first().components.first().items.first().itemLabel, QStringLiteral("TREE-ITEM"));
}

} // namespace

QTEST_APPLESS_MAIN(StructureServiceTest)

#include "structure_service_test.moc"
