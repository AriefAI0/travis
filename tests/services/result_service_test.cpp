#include <QtTest>

#include <QSqlQuery>

#include "data/repositories/inspection_type_repository.h"
#include "data/repositories/result_image_repository.h"
#include "data/repositories/result_repository.h"
#include "services/result_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class ResultServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema used by result service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies default inspection types are seeded once and can be queried.
    void ensureDefaultInspectionTypes_createsNamedRows();
    // Verifies result creation normalizes optional text fields.
    void createResult_persistsNormalizedFields();
    // Verifies result-image summary projection returns lightweight media rows.
    void resultImageSummaries_returnProjectedRows();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void ResultServiceTest::init() {
    connectionName_ = QStringLiteral("result_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");

    QSqlQuery query(database_);
    QVERIFY(query.exec("INSERT INTO project (title) VALUES ('Project One')"));
    QVERIFY(query.exec("INSERT INTO session (project_id, name) VALUES (1, 'Session One')"));
    QVERIFY(query.exec("INSERT INTO asset (project_id, name) VALUES (1, 'Asset One')"));
    QVERIFY(query.exec("INSERT INTO component (asset_id, name) VALUES (1, 'Component One')"));
    QVERIFY(query.exec("INSERT INTO item (component_id, item_label) VALUES (1, 'ITEM-001')"));
    QVERIFY(query.exec("INSERT INTO session_item (session_id, item_id) VALUES (1, 1)"));
    QVERIFY(query.exec("INSERT INTO execution_unit (type, name) VALUES ('ROV', 'ROV A')"));
}

void ResultServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void ResultServiceTest::ensureDefaultInspectionTypes_createsNamedRows() {
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository(database_);
    travis::data::repositories::ResultRepository resultRepository(database_);
    travis::data::repositories::ResultImageRepository resultImageRepository(database_);
    travis::services::ResultService service(
        inspectionTypeRepository,
        resultRepository,
        resultImageRepository
    );

    service.ensureDefaultInspectionTypes();

    const QVector<travis::models::InspectionType> inspectionTypes = service.listInspectionTypes();
    QCOMPARE(inspectionTypes.size(), travis::services::ResultService::defaultInspectionTypeNames().size());

    const auto gvi = service.getInspectionTypeByName(QStringLiteral("GVI"));
    QVERIFY(gvi.has_value());
}

void ResultServiceTest::createResult_persistsNormalizedFields() {
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository(database_);
    travis::data::repositories::ResultRepository resultRepository(database_);
    travis::data::repositories::ResultImageRepository resultImageRepository(database_);
    travis::services::ResultService service(
        inspectionTypeRepository,
        resultRepository,
        resultImageRepository
    );

    service.ensureDefaultInspectionTypes();
    const auto inspectionType = service.getInspectionTypeByName(QStringLiteral("CVI"));
    QVERIFY(inspectionType.has_value());

    const auto createdResult = service.createResult({
        .sessionItemId = 1,
        .inspectionTypeId = inspectionType->inspectionTypeId,
        .executionUnitId = 1,
        .toolingId = std::nullopt,
        .value = QStringLiteral("  12.5mm  "),
        .status = QStringLiteral("  completed  "),
        .remarks = QStringLiteral("  OK  "),
        .configSnapshot = QStringLiteral("  {\"gain\":2}  "),
    });

    QVERIFY(createdResult.has_value());
    QCOMPARE(createdResult->value.value(), QStringLiteral("12.5mm"));
    QCOMPARE(createdResult->status.value(), QStringLiteral("completed"));
    QCOMPARE(createdResult->remarks.value(), QStringLiteral("OK"));
    QCOMPARE(createdResult->configSnapshot.value(), QStringLiteral("{\"gain\":2}"));
}

void ResultServiceTest::resultImageSummaries_returnProjectedRows() {
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository(database_);
    travis::data::repositories::ResultRepository resultRepository(database_);
    travis::data::repositories::ResultImageRepository resultImageRepository(database_);
    travis::services::ResultService service(
        inspectionTypeRepository,
        resultRepository,
        resultImageRepository
    );

    service.ensureDefaultInspectionTypes();
    const auto inspectionType = service.getInspectionTypeByName(QStringLiteral("MGI"));
    QVERIFY(inspectionType.has_value());

    const auto createdResult = service.createResult({
        .sessionItemId = 1,
        .inspectionTypeId = inspectionType->inspectionTypeId,
        .executionUnitId = 1,
        .toolingId = std::nullopt,
        .value = std::nullopt,
        .status = QStringLiteral("completed"),
        .remarks = std::nullopt,
        .configSnapshot = std::nullopt,
    });

    QVERIFY(createdResult.has_value());

    const auto createdImage = service.createResultImage({
        .resultId = createdResult->resultId,
        .rawUrl = QStringLiteral("  raw/image-1.png  "),
        .annotatedUrl = QStringLiteral("  annotated/image-1.png  "),
        .remarks = QStringLiteral("  crack visible  "),
    });

    QVERIFY(createdImage.has_value());

    const QVector<travis::services::ResultImageSummary> summaries =
        service.listResultImageSummariesByResultId(createdResult->resultId);

    QCOMPARE(summaries.size(), 1);
    QCOMPARE(summaries.first().rawUrl, QStringLiteral("raw/image-1.png"));
    QCOMPARE(summaries.first().annotatedUrl.value(), QStringLiteral("annotated/image-1.png"));
    QCOMPARE(summaries.first().remarks.value(), QStringLiteral("crack visible"));
}

} // namespace

QTEST_GUILESS_MAIN(ResultServiceTest)

#include "result_service_test.moc"
