#include <QtTest>

#include "mediaEngine/playback/playback_engine.h"
#include "mediaEngine/thumbnail/thumbnail_generator.h"

namespace {

class PlaybackThumbnailEngineTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies media-engine playback and thumbnail boundaries reject invalid input explicitly.
    void validation_returnsStructuredFailures();
};

void PlaybackThumbnailEngineTest::validation_returnsStructuredFailures() {
    travis::media_engine::playback::PlaybackEngine playbackEngine;

    const auto missingPlaybackFile = playbackEngine.startFilePlayback("", nullptr);
    QVERIFY(!missingPlaybackFile.ok);
    QVERIFY(!missingPlaybackFile.message.empty());

    travis::media_engine::thumbnail::ThumbnailGenerator thumbnailGenerator;

    const auto missingMediaPath = thumbnailGenerator.validateRequest({
        .mediaFilePath = QString{},
        .outputImagePath = QStringLiteral("C:/tmp/thumb.jpg"),
        .timestampMs = 0,
        .width = 160,
        .height = 90,
    });
    QVERIFY(!missingMediaPath.ok);
    QVERIFY(!missingMediaPath.message.isEmpty());

    const auto invalidDimensions = thumbnailGenerator.validateRequest({
        .mediaFilePath = QCoreApplication::applicationFilePath(),
        .outputImagePath = QStringLiteral("C:/tmp/thumb.jpg"),
        .timestampMs = 0,
        .width = 0,
        .height = 90,
    });
    QVERIFY(!invalidDimensions.ok);
    QVERIFY(!invalidDimensions.message.isEmpty());
}

} // namespace

QTEST_GUILESS_MAIN(PlaybackThumbnailEngineTest)

#include "playback_thumbnail_engine_test.moc"
