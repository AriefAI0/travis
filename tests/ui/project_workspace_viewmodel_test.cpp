#include <QtTest>

#include "data/repositories/asset_repository.h"
#include "data/repositories/component_repository.h"
#include "data/repositories/item_repository.h"
#include "data/repositories/project_repository.h"
#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "services/project_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"
#include "support/sqlite_test_helper.h"
#include "ui/viewmodels/project_workspace_viewmodel.h"

namespace {

class ProjectWorkspaceViewModelTest : public QObject {
    Q_OBJECT

private slots:
    // Creates a full in-memory workspace graph for each test.
    void init();
    // Closes the per-test SQLite connection.
    void cleanup();

    // Verifies selected-project overview state is loaded from native services.
    void loadProject_exposesProjectStructureAndSessions();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void ProjectWorkspaceViewModelTest::init() {
    connectionName_ = QStringLiteral("project_workspace_viewmodel_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");
}

void ProjectWorkspaceViewModelTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void ProjectWorkspaceViewModelTest::loadProject_exposesProjectStructureAndSessions() {
    travis::data::repositories::ProjectRepository projectRepository(database_);
    travis::data::repositories::AssetRepository assetRepository(database_);
    travis::data::repositories::ComponentRepository componentRepository(database_);
    travis::data::repositories::ItemRepository itemRepository(database_);
    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);

    travis::services::ProjectService projectService(projectRepository);
    travis::services::StructureService structureService(assetRepository, componentRepository, itemRepository);
    travis::services::SessionService sessionService(sessionRepository, sessionItemRepository);

    const auto project = projectService.createProject({
        .title = QStringLiteral("Workspace Project"),
        .description = QStringLiteral("Workspace description"),
        .documentId = QStringLiteral("DOC-WORKSPACE"),
    });
    QVERIFY(project.has_value());

    const auto asset = structureService.createAsset({
        .projectId = project->projectId,
        .name = QStringLiteral("ROV"),
    });
    QVERIFY(asset.has_value());

    const auto component = structureService.createComponent({
        .assetId = asset->assetId,
        .name = QStringLiteral("Camera"),
    });
    QVERIFY(component.has_value());

    const auto item = structureService.createItem({
        .componentId = component->componentId,
        .itemLabel = QStringLiteral("CAM-001"),
        .position = QStringLiteral("Front"),
        .status = 1,
    });
    QVERIFY(item.has_value());

    const auto session = sessionService.createSession({
        .projectId = project->projectId,
        .name = QStringLiteral("Inspection Run 1"),
    });
    QVERIFY(session.has_value());

    travis::ui::viewmodels::ProjectWorkspaceViewModel viewModel(
        projectService,
        structureService,
        sessionService
    );

    QVERIFY(viewModel.loadProject(project->projectId));
    QCOMPARE(viewModel.projectId(), project->projectId);
    QCOMPARE(viewModel.project().value(QStringLiteral("title")).toString(), QStringLiteral("Workspace Project"));
    QCOMPARE(viewModel.assetCount(), 1);
    QCOMPARE(viewModel.componentCount(), 1);
    QCOMPARE(viewModel.itemCount(), 1);
    QCOMPARE(viewModel.structureTree().size(), 1);
    QCOMPARE(viewModel.sessions().size(), 1);
}

} // namespace

QTEST_GUILESS_MAIN(ProjectWorkspaceViewModelTest)

#include "project_workspace_viewmodel_test.moc"
