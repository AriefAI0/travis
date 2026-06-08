#include <QtTest>

#include "data/repositories/asset_repository.h"
#include "data/repositories/component_repository.h"
#include "data/repositories/execution_unit_repository.h"
#include "data/repositories/inspection_type_repository.h"
#include "data/repositories/item_repository.h"
#include "data/repositories/master_video_repository.h"
#include "data/repositories/result_image_repository.h"
#include "data/repositories/result_repository.h"
#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/tooling_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "services/execution_service.h"
#include "services/inspection_clip_service.h"
#include "services/result_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"
#include "services/video_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class InspectionClipServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema used by inspection-clip service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies starting a clip creates both result and recording clip rows.
    void startInspectionClip_createsLifecycle();
    // Verifies stopping a clip completes both the result and the clip.
    void stopInspectionClip_completesLifecycle();
    // Verifies cancel removes the active clip lifecycle.
    void cancelInspectionClip_removesLifecycle();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void InspectionClipServiceTest::init() {
    connectionName_ = QStringLiteral("inspection_clip_service_test_connection");
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
    QVERIFY(query.exec("INSERT INTO inspection_type (name) VALUES ('GVI')"));
    QVERIFY(query.exec("INSERT INTO execution_unit (type, name) VALUES ('ROV', 'ROV A')"));
    QVERIFY(query.exec("INSERT INTO master_video (session_id, file_url, start_epoch, end_epoch, status) VALUES (1, 'C:/videos/master.mkv', 1000, 1100, 'completed')"));
}

void InspectionClipServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void InspectionClipServiceTest::startInspectionClip_createsLifecycle() {
    travis::services::SessionService sessionService(
        travis::data::repositories::SessionRepository(database_),
        travis::data::repositories::SessionItemRepository(database_)
    );
    travis::services::StructureService structureService(
        travis::data::repositories::AssetRepository(database_),
        travis::data::repositories::ComponentRepository(database_),
        travis::data::repositories::ItemRepository(database_)
    );
    travis::services::ExecutionService executionService(
        travis::data::repositories::ExecutionUnitRepository(database_),
        travis::data::repositories::ToolingRepository(database_)
    );
    travis::services::ResultService resultService(
        travis::data::repositories::InspectionTypeRepository(database_),
        travis::data::repositories::ResultRepository(database_),
        travis::data::repositories::ResultImageRepository(database_)
    );
    travis::services::VideoService videoService(
        travis::data::repositories::MasterVideoRepository(database_),
        travis::data::repositories::VideoClipRepository(database_),
        travis::data::repositories::TimelineThumbnailRepository(database_)
    );
    travis::services::InspectionClipService service(
        resultService,
        videoService,
        sessionService,
        structureService,
        executionService
    );

    const auto lifecycle = service.startInspectionClip({
        .sessionItemId = 1,
        .inspectionTypeId = 1,
        .executionUnitId = 1,
        .toolingId = std::nullopt,
        .masterVideoId = 1,
        .startOffsetMs = 5000,
        .value = QStringLiteral("  value  "),
        .remarks = QStringLiteral("  remark  "),
        .configSnapshot = QStringLiteral("  {}  "),
    });

    QCOMPARE(lifecycle.result.sessionItemId, 1);
    QCOMPARE(lifecycle.result.status.value(), travis::services::InspectionClipService::inProgressStatus());
    QCOMPARE(lifecycle.clip.masterVideoId, 1);
    QCOMPARE(lifecycle.clip.status, QStringLiteral("recording"));
}

void InspectionClipServiceTest::stopInspectionClip_completesLifecycle() {
    travis::services::SessionService sessionService(
        travis::data::repositories::SessionRepository(database_),
        travis::data::repositories::SessionItemRepository(database_)
    );
    travis::services::StructureService structureService(
        travis::data::repositories::AssetRepository(database_),
        travis::data::repositories::ComponentRepository(database_),
        travis::data::repositories::ItemRepository(database_)
    );
    travis::services::ExecutionService executionService(
        travis::data::repositories::ExecutionUnitRepository(database_),
        travis::data::repositories::ToolingRepository(database_)
    );
    travis::services::ResultService resultService(
        travis::data::repositories::InspectionTypeRepository(database_),
        travis::data::repositories::ResultRepository(database_),
        travis::data::repositories::ResultImageRepository(database_)
    );
    travis::services::VideoService videoService(
        travis::data::repositories::MasterVideoRepository(database_),
        travis::data::repositories::VideoClipRepository(database_),
        travis::data::repositories::TimelineThumbnailRepository(database_)
    );
    travis::services::InspectionClipService service(
        resultService,
        videoService,
        sessionService,
        structureService,
        executionService
    );

    const auto started = service.startInspectionClip({
        .sessionItemId = 1,
        .inspectionTypeId = 1,
        .executionUnitId = 1,
        .toolingId = std::nullopt,
        .masterVideoId = 1,
        .startOffsetMs = 1000,
        .value = std::nullopt,
        .remarks = std::nullopt,
        .configSnapshot = std::nullopt,
    });

    const auto completed = service.stopInspectionClip({
        .clipId = started.clip.clipId,
        .endOffsetMs = 4000,
        .thumbnailUrl = QStringLiteral("  C:/clips/thumb.png  "),
        .remarks = QStringLiteral("  completed remark  "),
    });

    QVERIFY(completed.has_value());
    QCOMPARE(completed->result.status.value(), travis::services::InspectionClipService::completedStatus());
    QCOMPARE(completed->clip.endOffsetMs.value(), 4000);
    QCOMPARE(completed->clip.thumbnailUrl.value(), QStringLiteral("C:/clips/thumb.png"));
    QCOMPARE(completed->clip.status, QStringLiteral("completed"));
}

void InspectionClipServiceTest::cancelInspectionClip_removesLifecycle() {
    travis::services::SessionService sessionService(
        travis::data::repositories::SessionRepository(database_),
        travis::data::repositories::SessionItemRepository(database_)
    );
    travis::services::StructureService structureService(
        travis::data::repositories::AssetRepository(database_),
        travis::data::repositories::ComponentRepository(database_),
        travis::data::repositories::ItemRepository(database_)
    );
    travis::services::ExecutionService executionService(
        travis::data::repositories::ExecutionUnitRepository(database_),
        travis::data::repositories::ToolingRepository(database_)
    );
    travis::services::ResultService resultService(
        travis::data::repositories::InspectionTypeRepository(database_),
        travis::data::repositories::ResultRepository(database_),
        travis::data::repositories::ResultImageRepository(database_)
    );
    travis::services::VideoService videoService(
        travis::data::repositories::MasterVideoRepository(database_),
        travis::data::repositories::VideoClipRepository(database_),
        travis::data::repositories::TimelineThumbnailRepository(database_)
    );
    travis::services::InspectionClipService service(
        resultService,
        videoService,
        sessionService,
        structureService,
        executionService
    );

    const auto started = service.startInspectionClip({
        .sessionItemId = 1,
        .inspectionTypeId = 1,
        .executionUnitId = 1,
        .toolingId = std::nullopt,
        .masterVideoId = 1,
        .startOffsetMs = 2000,
        .value = std::nullopt,
        .remarks = std::nullopt,
        .configSnapshot = std::nullopt,
    });

    const auto cancelled = service.cancelInspectionClip(started.clip.clipId);
    QVERIFY(cancelled.has_value());
    QCOMPARE(cancelled->clip.clipId, started.clip.clipId);
    QVERIFY(!videoService.getVideoClipById(started.clip.clipId).has_value());
    QVERIFY(!resultService.getResultById(started.result.resultId).has_value());
}

} // namespace

QTEST_APPLESS_MAIN(InspectionClipServiceTest)

#include "inspection_clip_service_test.moc"
