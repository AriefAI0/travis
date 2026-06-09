#include <QtTest>

#include "ui/controllers/playback_surface_controller.h"

namespace {

class PlaybackSurfaceControllerTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies playback surface validation reports errors before touching a media file.
    void startPlayback_withoutSurface_reportsError();
};

void PlaybackSurfaceControllerTest::startPlayback_withoutSurface_reportsError() {
    travis::ui::controllers::PlaybackSurfaceController controller;

    QVERIFY(!controller.startPlayback(QStringLiteral("C:/tmp/missing.mkv")));
    QVERIFY(!controller.lastError().isEmpty());
    QVERIFY(!controller.playbackActive());
}

} // namespace

QTEST_GUILESS_MAIN(PlaybackSurfaceControllerTest)

#include "playback_surface_controller_test.moc"
