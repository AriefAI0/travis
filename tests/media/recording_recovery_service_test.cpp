#include <QtTest>

#include <QDir>
#include <QFile>
#include <QSqlQuery>

#include "application/recovery/recording_recovery_service.h"
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

class RecordingRecoveryServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema needed by the recovery workflow smoke tests.
    void init();
    // Closes and removes the per-test database connection and temp files.
    void cleanup();

    // Verifies interrupted rows are reconciled using the persisted file presence policy.
    void recoverInterruptedRecordings_reconcilesMasterVideosAndClips();

private:
    QString connectionName_;
    QSqlDatabase database_;
    QString existingMasterVideoPath_;
};

void RecordingRecoveryServiceTest::init() {
    connectionName_ = QStringLiteral("recording_recovery_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");

    const QString tempDir = QDir::temp().filePath(QStringLiteral("travis-media-tests"));
    QVERIFY(QDir().mkpath(tempDir));
    existingMasterVideoPath_ = QDir(tempDir).filePath(QStringLiteral("recoverable-master.mkv"));

    QFile file(existingMasterVideoPath_);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("mkv") > 0);
    file.close();
}

void RecordingRecoveryServiceTest::cleanup() {
    QFile::remove(existingMasterVideoPath_);
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void RecordingRecoveryServiceTest::recoverInterruptedRecordings_reconcilesMasterVideosAndClips() {
    QSqlQuery query(database_);
    QVERIFY(query.exec(QStringLiteral("INSERT INTO project (project_id, title) VALUES (1, 'Project')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO session (session_id, project_id, name) VALUES (1, 1, 'Session')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO inspection_type (inspection_type_id, name) VALUES (1, 'GVI')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO execution_unit (execution_unit_id, type, name) VALUES (1, 'default', 'Main')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO asset (asset_id, project_id, name) VALUES (1, 1, 'Asset')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO component (component_id, asset_id, name) VALUES (1, 1, 'Component')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO item (item_id, component_id, item_label) VALUES (1, 1, 'Item')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO session_item (session_item_id, session_id, item_id) VALUES (1, 1, 1)")));
    QVERIFY(query.exec(QStringLiteral(R"(
        INSERT INTO result (result_id, session_item_id, inspection_type_id, execution_unit_id, status)
        VALUES (1, 1, 1, 1, 'in_progress')
    )")));
    QVERIFY(query.exec(QStringLiteral(R"(
        INSERT INTO master_video (master_video_id, session_id, file_url, start_epoch, status)
        VALUES (1, 1, '%1', 100, 'recording')
    )").arg(existingMasterVideoPath_)));
    QVERIFY(query.exec(QStringLiteral(R"(
        INSERT INTO video_clip (clip_id, result_id, master_video_id, start_offset_ms, status)
        VALUES (1, 1, 1, 1000, 'recording')
    )")));

    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);
    travis::data::repositories::AssetRepository assetRepository(database_);
    travis::data::repositories::ComponentRepository componentRepository(database_);
    travis::data::repositories::ItemRepository itemRepository(database_);
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository(database_);
    travis::data::repositories::ToolingRepository toolingRepository(database_);
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository(database_);
    travis::data::repositories::ResultRepository resultRepository(database_);
    travis::data::repositories::ResultImageRepository resultImageRepository(database_);
    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);

    travis::services::SessionService sessionService{
        sessionRepository,
        sessionItemRepository
    };
    travis::services::StructureService structureService{
        assetRepository,
        componentRepository,
        itemRepository
    };
    travis::services::ExecutionService executionService{
        executionUnitRepository,
        toolingRepository
    };
    travis::services::ResultService resultService{
        inspectionTypeRepository,
        resultRepository,
        resultImageRepository
    };
    travis::services::VideoService videoService{
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    };
    travis::services::InspectionClipService inspectionClipService{
        resultService,
        videoService,
        sessionService,
        structureService,
        executionService
    };
    travis::application::recovery::RecordingRecoveryService recoveryService{
        videoService,
        inspectionClipService,
        resultService
    };

    const auto report = recoveryService.recoverInterruptedRecordings();
    QCOMPARE(report.recoveredMasterVideos.size(), 1);
    QCOMPARE(report.recoveredMasterVideos.first().action, QStringLiteral("completed_from_existing_file"));
    QCOMPARE(report.recoveredVideoClips.size(), 1);
    QCOMPARE(report.recoveredVideoClips.first().action, QStringLiteral("cancelled_missing_or_empty_file"));
}

} // namespace

QTEST_GUILESS_MAIN(RecordingRecoveryServiceTest)

#include "recording_recovery_service_test.moc"
