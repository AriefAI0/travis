#include <QtTest>

#include "data/repositories/asset_repository.h"
#include "data/repositories/component_repository.h"
#include "data/repositories/item_repository.h"
#include "data/repositories/project_repository.h"
#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "data/repositories/master_video_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "services/project_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"
#include "services/video_service.h"
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
    // Verifies structure creation actions refresh the loaded workspace snapshot.
    void createStructureNodes_refreshesWorkspace();

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
    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);

    travis::services::ProjectService projectService(projectRepository);
    travis::services::StructureService structureService(assetRepository, componentRepository, itemRepository);
    travis::services::SessionService sessionService(sessionRepository, sessionItemRepository);
    travis::services::VideoService videoService(
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    );

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

    const auto masterVideo = videoService.createMasterVideo({
        .sessionId = session->sessionId,
        .fileUrl = QStringLiteral("C:/recordings/master-1.mkv"),
        .thumbnailUrl = std::nullopt,
        .startEpoch = 1000,
        .endEpoch = std::nullopt,
        .status = QStringLiteral("recording"),
        .sourceName = QStringLiteral("Camera 1"),
    });
    QVERIFY(masterVideo.has_value());

    travis::ui::viewmodels::ProjectWorkspaceViewModel viewModel(
        projectService,
        structureService,
        sessionService,
        videoService
    );

    QVERIFY(viewModel.loadProject(project->projectId));
    QCOMPARE(viewModel.projectId(), project->projectId);
    QCOMPARE(viewModel.project().value(QStringLiteral("title")).toString(), QStringLiteral("Workspace Project"));
    QCOMPARE(viewModel.assetCount(), 1);
    QCOMPARE(viewModel.componentCount(), 1);
    QCOMPARE(viewModel.itemCount(), 1);
    QCOMPARE(viewModel.structureTree().size(), 1);
    QCOMPARE(viewModel.sessions().size(), 1);
    QCOMPARE(viewModel.masterVideos().size(), 1);
    QCOMPARE(
        viewModel.masterVideos().first().toMap().value(QStringLiteral("fileUrl")).toString(),
        QStringLiteral("C:/recordings/master-1.mkv")
    );
}

void ProjectWorkspaceViewModelTest::createStructureNodes_refreshesWorkspace() {
    travis::data::repositories::ProjectRepository projectRepository(database_);
    travis::data::repositories::AssetRepository assetRepository(database_);
    travis::data::repositories::ComponentRepository componentRepository(database_);
    travis::data::repositories::ItemRepository itemRepository(database_);
    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);
    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);

    travis::services::ProjectService projectService(projectRepository);
    travis::services::StructureService structureService(assetRepository, componentRepository, itemRepository);
    travis::services::SessionService sessionService(sessionRepository, sessionItemRepository);
    travis::services::VideoService videoService(
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    );

    const auto project = projectService.createProject({
        .title = QStringLiteral("Editable Workspace Project"),
        .description = std::nullopt,
        .documentId = std::nullopt,
    });
    QVERIFY(project.has_value());

    travis::ui::viewmodels::ProjectWorkspaceViewModel viewModel(
        projectService,
        structureService,
        sessionService,
        videoService
    );

    QVERIFY(viewModel.loadProject(project->projectId));
    QVERIFY(viewModel.createAsset(QStringLiteral("Hull")));
    QCOMPARE(viewModel.assetCount(), 1);

    const QVariantMap asset = viewModel.structureTree().first().toMap();
    const qint64 assetId = asset.value(QStringLiteral("assetId")).toLongLong();

    QVERIFY(viewModel.createComponent(assetId, QStringLiteral("Anode")));
    QCOMPARE(viewModel.componentCount(), 1);

    const QVariantList components = viewModel.structureTree().first().toMap()
        .value(QStringLiteral("components")).toList();
    const qint64 componentId = components.first().toMap().value(QStringLiteral("componentId")).toLongLong();

    QVERIFY(viewModel.createItem(
        componentId,
        QStringLiteral("ANODE-001"),
        QStringLiteral("Port"),
        1
    ));
    QCOMPARE(viewModel.itemCount(), 1);
}

} // namespace

QTEST_GUILESS_MAIN(ProjectWorkspaceViewModelTest)

#include "project_workspace_viewmodel_test.moc"
