#include <QtTest>

#include "data/repositories/project_repository.h"
#include "services/project_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class ProjectServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema needed by the project service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies project creation trims text fields and persists the row.
    void createProject_persistsNormalizedValues();
    // Verifies blank titles are rejected before repository execution.
    void createProject_rejectsBlankTitle();
    // Verifies project updates normalize optional text fields.
    void updateProject_normalizesOptionalFields();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void ProjectServiceTest::init() {
    connectionName_ = QStringLiteral("project_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");
}

void ProjectServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void ProjectServiceTest::createProject_persistsNormalizedValues() {
    travis::data::repositories::ProjectRepository repository(database_);
    travis::services::ProjectService service(repository);

    const auto createdProject = service.createProject({
        .title = QStringLiteral("  Travis Project  "),
        .description = QStringLiteral("  Demo description  "),
        .documentId = QStringLiteral("  DOC-001  "),
    });

    QVERIFY(createdProject.has_value());
    QCOMPARE(createdProject->title, QStringLiteral("Travis Project"));
    QCOMPARE(createdProject->description.value(), QStringLiteral("Demo description"));
    QCOMPARE(createdProject->documentId.value(), QStringLiteral("DOC-001"));

    const QVector<travis::models::Project> projects = service.listProjects();
    QCOMPARE(projects.size(), 1);
    QCOMPARE(projects.first().title, QStringLiteral("Travis Project"));
}

void ProjectServiceTest::createProject_rejectsBlankTitle() {
    travis::data::repositories::ProjectRepository repository(database_);
    travis::services::ProjectService service(repository);

    QVERIFY_EXCEPTION_THROWN(
        service.createProject({
            .title = QStringLiteral("   "),
            .description = std::nullopt,
            .documentId = std::nullopt,
        }),
        std::runtime_error
    );
}

void ProjectServiceTest::updateProject_normalizesOptionalFields() {
    travis::data::repositories::ProjectRepository repository(database_);
    travis::services::ProjectService service(repository);

    const auto createdProject = service.createProject({
        .title = QStringLiteral("Project Alpha"),
        .description = QStringLiteral("Description"),
        .documentId = QStringLiteral("DOC-ALPHA"),
    });

    QVERIFY(createdProject.has_value());

    const auto updatedProject = service.updateProject(
        createdProject->projectId,
        {
            .title = QStringLiteral("  Project Beta  "),
            .description = QStringLiteral("   "),
            .documentId = QStringLiteral("  DOC-BETA  "),
        }
    );

    QVERIFY(updatedProject.has_value());
    QCOMPARE(updatedProject->title, QStringLiteral("Project Beta"));
    QVERIFY(!updatedProject->description.has_value());
    QCOMPARE(updatedProject->documentId.value(), QStringLiteral("DOC-BETA"));
}

} // namespace

QTEST_APPLESS_MAIN(ProjectServiceTest)

#include "project_service_test.moc"
