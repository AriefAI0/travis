#include <QtTest>

#include "data/repositories/project_repository.h"
#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "services/project_service.h"
#include "services/session_service.h"
#include "support/sqlite_test_helper.h"
#include "ui/viewmodels/inspection_context_viewmodel.h"

namespace {

class InspectionContextViewModelTest : public QObject {
    Q_OBJECT

private slots:
    // Creates a clean project/session schema for each test.
    void init();
    // Closes the per-test SQLite connection.
    void cleanup();

    // Verifies project context loads sessions and can create/select an inspection session.
    void loadProjectAndCreateSession_updatesSelectedSession();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void InspectionContextViewModelTest::init() {
    connectionName_ = QStringLiteral("inspection_context_viewmodel_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");
}

void InspectionContextViewModelTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void InspectionContextViewModelTest::loadProjectAndCreateSession_updatesSelectedSession() {
    travis::data::repositories::ProjectRepository projectRepository(database_);
    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);
    travis::services::ProjectService projectService(projectRepository);
    travis::services::SessionService sessionService(sessionRepository, sessionItemRepository);

    const auto project = projectService.createProject({
        .title = QStringLiteral("Inspection Project"),
        .description = std::nullopt,
        .documentId = std::nullopt,
    });
    QVERIFY(project.has_value());

    travis::ui::viewmodels::InspectionContextViewModel viewModel(projectService, sessionService);

    QVERIFY(viewModel.loadProject(project->projectId));
    QCOMPARE(viewModel.projectId(), project->projectId);
    QCOMPARE(viewModel.sessions().size(), 0);

    QVERIFY(viewModel.createSession(QStringLiteral("Dive 001")));
    QVERIFY(viewModel.sessionId() > 0);
    QCOMPARE(viewModel.sessions().size(), 1);
    QCOMPARE(viewModel.selectedSession().value(QStringLiteral("name")).toString(), QStringLiteral("Dive 001"));

    const qint64 createdSessionId = viewModel.sessionId();
    QVERIFY(viewModel.selectSession(createdSessionId));
    QCOMPARE(viewModel.sessionId(), createdSessionId);
}

} // namespace

QTEST_GUILESS_MAIN(InspectionContextViewModelTest)

#include "inspection_context_viewmodel_test.moc"
