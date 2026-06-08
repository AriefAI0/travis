#include <QtTest>

#include "data/repositories/master_video_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "services/video_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class VideoServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema used by video service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies master-video creation normalizes text fields and time range.
    void createMasterVideo_persistsNormalizedFields();
    // Verifies clip playback is derived from clip and master-video rows.
    void getVideoClipPlaybackById_returnsDerivedPlayback();
    // Verifies timeline thumbnail replacement rewrites one master-video thumbnail set.
    void replaceMasterVideoTimelineThumbnails_rewritesThumbnailSet();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void VideoServiceTest::init() {
    connectionName_ = QStringLiteral("video_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");

    QSqlQuery query(database_);
    QVERIFY(query.exec("INSERT INTO project (title) VALUES ('Project One')"));
    QVERIFY(query.exec("INSERT INTO session (project_id, name) VALUES (1, 'Session One')"));
    QVERIFY(query.exec("INSERT INTO execution_unit (type, name) VALUES ('ROV', 'ROV A')"));
    QVERIFY(query.exec("INSERT INTO inspection_type (name) VALUES ('GVI')"));
    QVERIFY(query.exec("INSERT INTO asset (project_id, name) VALUES (1, 'Asset One')"));
    QVERIFY(query.exec("INSERT INTO component (asset_id, name) VALUES (1, 'Component One')"));
    QVERIFY(query.exec("INSERT INTO item (component_id, item_label) VALUES (1, 'ITEM-001')"));
    QVERIFY(query.exec("INSERT INTO session_item (session_id, item_id) VALUES (1, 1)"));
    QVERIFY(query.exec(
        "INSERT INTO result (session_item_id, inspection_type_id, execution_unit_id, status) "
        "VALUES (1, 1, 1, 'in_progress')"
    ));
}

void VideoServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void VideoServiceTest::createMasterVideo_persistsNormalizedFields() {
    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);
    travis::services::VideoService service(
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    );

    const auto createdMasterVideo = service.createMasterVideo({
        .sessionId = 1,
        .fileUrl = QStringLiteral("  C:/videos/master-1.mkv  "),
        .thumbnailUrl = QStringLiteral("  C:/thumbs/master-1.png  "),
        .startEpoch = 1000,
        .endEpoch = 1060,
        .status = QStringLiteral("completed"),
        .sourceName = QStringLiteral("  Camera 1  "),
    });

    QVERIFY(createdMasterVideo.has_value());
    QCOMPARE(createdMasterVideo->fileUrl, QStringLiteral("C:/videos/master-1.mkv"));
    QCOMPARE(createdMasterVideo->thumbnailUrl.value(), QStringLiteral("C:/thumbs/master-1.png"));
    QCOMPARE(createdMasterVideo->sourceName.value(), QStringLiteral("Camera 1"));
}

void VideoServiceTest::getVideoClipPlaybackById_returnsDerivedPlayback() {
    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);
    travis::services::VideoService service(
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    );

    const auto createdMasterVideo = service.createMasterVideo({
        .sessionId = 1,
        .fileUrl = QStringLiteral("C:/videos/master-2.mkv"),
        .thumbnailUrl = std::nullopt,
        .startEpoch = 2000,
        .endEpoch = 2100,
        .status = QStringLiteral("completed"),
        .sourceName = QStringLiteral("Camera 2"),
    });

    QVERIFY(createdMasterVideo.has_value());

    const auto createdClip = service.createVideoClip({
        .resultId = 1,
        .masterVideoId = createdMasterVideo->masterVideoId,
        .startOffsetMs = 5000,
        .endOffsetMs = 9000,
        .clipFileUrl = QStringLiteral("C:/clips/clip-1.mkv"),
        .thumbnailUrl = QStringLiteral("C:/clips/clip-1.png"),
        .status = QStringLiteral("completed"),
    });

    QVERIFY(createdClip.has_value());

    const auto playback = service.getVideoClipPlaybackById(createdClip->clipId);
    QVERIFY(playback.has_value());
    QCOMPARE(playback->fileUrl, QStringLiteral("C:/videos/master-2.mkv"));
    QCOMPARE(playback->durationMs.value(), 4000);
    QCOMPARE(playback->startEpochMs, 2000 * 1000 + 5000);
    QCOMPARE(playback->endEpochMs.value(), 2000 * 1000 + 9000);
}

void VideoServiceTest::replaceMasterVideoTimelineThumbnails_rewritesThumbnailSet() {
    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);
    travis::services::VideoService service(
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    );

    const auto createdMasterVideo = service.createMasterVideo({
        .sessionId = 1,
        .fileUrl = QStringLiteral("C:/videos/master-3.mkv"),
        .thumbnailUrl = std::nullopt,
        .startEpoch = 3000,
        .endEpoch = 3060,
        .status = QStringLiteral("completed"),
        .sourceName = std::nullopt,
    });

    QVERIFY(createdMasterVideo.has_value());

    const QVector<travis::models::TimelineThumbnail> firstBatch =
        service.replaceMasterVideoTimelineThumbnails(
            createdMasterVideo->masterVideoId,
            {
                {
                    .masterVideoId = createdMasterVideo->masterVideoId,
                    .timestampMs = 1000,
                    .imagePath = QStringLiteral("C:/thumbs/t1.png"),
                    .width = 320,
                    .height = 180,
                    .sizeBytes = 1000,
                },
                {
                    .masterVideoId = createdMasterVideo->masterVideoId,
                    .timestampMs = 2000,
                    .imagePath = QStringLiteral("C:/thumbs/t2.png"),
                    .width = 320,
                    .height = 180,
                    .sizeBytes = 1100,
                },
            }
        );

    QCOMPARE(firstBatch.size(), 2);

    const QVector<travis::models::TimelineThumbnail> secondBatch =
        service.replaceMasterVideoTimelineThumbnails(
            createdMasterVideo->masterVideoId,
            {
                {
                    .masterVideoId = createdMasterVideo->masterVideoId,
                    .timestampMs = 3000,
                    .imagePath = QStringLiteral("C:/thumbs/t3.png"),
                    .width = 320,
                    .height = 180,
                    .sizeBytes = 1200,
                },
            }
        );

    QCOMPARE(secondBatch.size(), 1);

    const QVector<travis::models::TimelineThumbnail> thumbnails =
        service.listMasterVideoTimelineThumbnailsByMasterVideoId(createdMasterVideo->masterVideoId);
    QCOMPARE(thumbnails.size(), 1);
    QCOMPARE(thumbnails.first().imagePath, QStringLiteral("C:/thumbs/t3.png"));
}

} // namespace

QTEST_APPLESS_MAIN(VideoServiceTest)

#include "video_service_test.moc"
