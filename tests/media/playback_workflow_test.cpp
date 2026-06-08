#include <QtTest>

#include <QSqlQuery>

#include "application/playback/playback_workflow_service.h"
#include "data/repositories/inspection_type_repository.h"
#include "data/repositories/master_video_repository.h"
#include "data/repositories/result_image_repository.h"
#include "data/repositories/result_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "services/result_media_service.h"
#include "services/result_service.h"
#include "services/video_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class PlaybackWorkflowTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema needed by the playback workflow smoke tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies the playback workflow joins persisted clip and image data.
    void openVideoClip_returnsPlaybackSummary();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void PlaybackWorkflowTest::init() {
    connectionName_ = QStringLiteral("playback_workflow_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");
}

void PlaybackWorkflowTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void PlaybackWorkflowTest::openVideoClip_returnsPlaybackSummary() {
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
        VALUES (1, 1, 1, 1, 'completed')
    )")));
    QVERIFY(query.exec(QStringLiteral(R"(
        INSERT INTO master_video (master_video_id, session_id, file_url, start_epoch, end_epoch, status)
        VALUES (1, 1, 'C:/tmp/master.mkv', 100, 110, 'completed')
    )")));
    QVERIFY(query.exec(QStringLiteral(R"(
        INSERT INTO video_clip (clip_id, result_id, master_video_id, start_offset_ms, end_offset_ms, clip_file_url, status)
        VALUES (1, 1, 1, 1000, 3000, 'C:/tmp/clip.mkv', 'completed')
    )")));
    QVERIFY(query.exec(QStringLiteral(R"(
        INSERT INTO result_image (image_id, result_id, raw_url, annotated_url)
        VALUES (1, 1, 'C:/tmp/raw.jpg', 'C:/tmp/annotated.jpg')
    )")));

    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);
    travis::data::repositories::ResultImageRepository resultImageRepository(database_);

    travis::services::VideoService videoService{
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    };
    travis::services::ResultMediaService resultMediaService{resultImageRepository};
    travis::application::playback::PlaybackWorkflowService playbackWorkflow{
        videoService,
        resultMediaService
    };

    const auto playback = playbackWorkflow.openVideoClip(1);
    QVERIFY(playback.has_value());
    QCOMPARE(playback->clipPlayback.clipId, 1);
    QCOMPARE(playback->clipPlayback.masterVideoId, 1);
    QCOMPARE(playback->resultImages.size(), 1);
    QCOMPARE(playback->resultImages.first().rawUrl, QStringLiteral("C:/tmp/raw.jpg"));
}

} // namespace

QTEST_GUILESS_MAIN(PlaybackWorkflowTest)

#include "playback_workflow_test.moc"
