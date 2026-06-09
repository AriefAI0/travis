#include <QtTest>

#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QTemporaryDir>

#include "application/thumbnail/thumbnail_workflow_service.h"
#include "data/repositories/master_video_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "services/video_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class ThumbnailWorkflowServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema needed by thumbnail workflow tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies validated thumbnail outputs replace persisted timeline thumbnail rows.
    void replaceMasterVideoTimelineThumbnails_persistsValidatedOutputs();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void ThumbnailWorkflowServiceTest::init() {
    connectionName_ = QStringLiteral("thumbnail_workflow_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");
}

void ThumbnailWorkflowServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void ThumbnailWorkflowServiceTest::replaceMasterVideoTimelineThumbnails_persistsValidatedOutputs() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("master.mkv"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    QVERIFY(mediaFile.write("media") > 0);
    mediaFile.close();

    const QString thumbnailPath = tempDir.filePath(QStringLiteral("thumb-0001.jpg"));
    QFile thumbnailFile(thumbnailPath);
    QVERIFY(thumbnailFile.open(QIODevice::WriteOnly));
    QVERIFY(thumbnailFile.write("thumbnail") > 0);
    thumbnailFile.close();

    QSqlQuery query(database_);
    QVERIFY(query.exec(QStringLiteral("INSERT INTO project (project_id, title) VALUES (1, 'Project')")));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO session (session_id, project_id, name) VALUES (1, 1, 'Session')")));

    travis::data::repositories::MasterVideoRepository masterVideoRepository(database_);
    travis::data::repositories::VideoClipRepository videoClipRepository(database_);
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository(database_);

    travis::services::VideoService videoService{
        masterVideoRepository,
        videoClipRepository,
        timelineThumbnailRepository
    };

    const auto masterVideo = videoService.createMasterVideo({
        .sessionId = 1,
        .fileUrl = mediaPath,
        .thumbnailUrl = std::nullopt,
        .startEpoch = 100,
        .endEpoch = std::nullopt,
        .status = QStringLiteral("completed"),
        .sourceName = std::nullopt,
    });
    QVERIFY(masterVideo.has_value());

    travis::application::thumbnail::ThumbnailWorkflowService workflow{videoService};
    const auto result = workflow.replaceMasterVideoTimelineThumbnails(
        masterVideo->masterVideoId,
        {
            {
                .timestampMs = 1000,
                .imagePath = thumbnailPath,
                .width = 160,
                .height = 90,
                .sizeBytes = thumbnailFile.size(),
            },
        }
    );

    QVERIFY2(result.ok, qPrintable(result.message));
    QCOMPARE(result.thumbnails.size(), 1);
    QCOMPARE(result.thumbnails.first().masterVideoId, masterVideo->masterVideoId);
    QCOMPARE(result.thumbnails.first().imagePath, thumbnailPath);
}

} // namespace

QTEST_GUILESS_MAIN(ThumbnailWorkflowServiceTest)

#include "thumbnail_workflow_service_test.moc"
