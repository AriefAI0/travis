#include <QtTest>

#include "data/repositories/execution_unit_repository.h"
#include "data/repositories/tooling_repository.h"
#include "services/execution_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class ExecutionServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema used by execution service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies execution-unit creation trims required and optional text fields.
    void createExecutionUnit_persistsNormalizedFields();
    // Verifies execution-unit lookups by type and name work as expected.
    void findExecutionUnitByTypeAndName_returnsRow();
    // Verifies tooling lifecycle operations use the linked execution unit.
    void toolingLifecycle_persistsNormalizedFields();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void ExecutionServiceTest::init() {
    connectionName_ = QStringLiteral("execution_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");
}

void ExecutionServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void ExecutionServiceTest::createExecutionUnit_persistsNormalizedFields() {
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository(database_);
    travis::data::repositories::ToolingRepository toolingRepository(database_);
    travis::services::ExecutionService service(executionUnitRepository, toolingRepository);

    const auto createdExecutionUnit = service.createExecutionUnit({
        .type = QStringLiteral("  ROV  "),
        .name = QStringLiteral("  ROV A  "),
        .meta = QStringLiteral("  {\"depth\":300}  "),
    });

    QVERIFY(createdExecutionUnit.has_value());
    QCOMPARE(createdExecutionUnit->type, QStringLiteral("ROV"));
    QCOMPARE(createdExecutionUnit->name, QStringLiteral("ROV A"));
    QCOMPARE(createdExecutionUnit->meta.value(), QStringLiteral("{\"depth\":300}"));
}

void ExecutionServiceTest::findExecutionUnitByTypeAndName_returnsRow() {
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository(database_);
    travis::data::repositories::ToolingRepository toolingRepository(database_);
    travis::services::ExecutionService service(executionUnitRepository, toolingRepository);

    const auto createdExecutionUnit = service.createExecutionUnit({
        .type = QStringLiteral("ROV"),
        .name = QStringLiteral("Survey Unit"),
        .meta = std::nullopt,
    });

    QVERIFY(createdExecutionUnit.has_value());

    const auto foundExecutionUnit = service.getExecutionUnitByTypeAndName(
        QStringLiteral("  ROV  "),
        QStringLiteral("  Survey Unit  ")
    );

    QVERIFY(foundExecutionUnit.has_value());
    QCOMPARE(foundExecutionUnit->executionUnitId, createdExecutionUnit->executionUnitId);
}

void ExecutionServiceTest::toolingLifecycle_persistsNormalizedFields() {
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository(database_);
    travis::data::repositories::ToolingRepository toolingRepository(database_);
    travis::services::ExecutionService service(executionUnitRepository, toolingRepository);

    const auto createdExecutionUnit = service.createExecutionUnit({
        .type = QStringLiteral("DIVER"),
        .name = QStringLiteral("Diver Unit"),
        .meta = std::nullopt,
    });

    QVERIFY(createdExecutionUnit.has_value());

    const auto createdTooling = service.createTooling({
        .executionUnitId = createdExecutionUnit->executionUnitId,
        .name = QStringLiteral("  Camera Rig  "),
        .config = QStringLiteral("  {\"mode\":\"4k\"}  "),
    });

    QVERIFY(createdTooling.has_value());
    QCOMPARE(createdTooling->name, QStringLiteral("Camera Rig"));
    QCOMPARE(createdTooling->config.value(), QStringLiteral("{\"mode\":\"4k\"}"));

    const auto updatedTooling = service.updateTooling(
        createdTooling->toolingId,
        {
            .name = QStringLiteral("  Camera Rig V2  "),
            .config = QStringLiteral("   "),
        }
    );

    QVERIFY(updatedTooling.has_value());
    QCOMPARE(updatedTooling->name, QStringLiteral("Camera Rig V2"));
    QVERIFY(!updatedTooling->config.has_value());

    const QVector<travis::models::Tooling> toolings =
        service.listToolingsByExecutionUnitId(createdExecutionUnit->executionUnitId);
    QCOMPARE(toolings.size(), 1);
}

} // namespace

QTEST_APPLESS_MAIN(ExecutionServiceTest)

#include "execution_service_test.moc"
