#include <QtTest>

#include "mediaEngine/discovery/source_discovery.h"

namespace {

class SourceDiscoveryTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies GStreamer device discovery can run through the bundled runtime.
    void listCaptureSources_returnsStructuredResults();
};

void SourceDiscoveryTest::listCaptureSources_returnsStructuredResults() {
    travis::media_engine::discovery::SourceDiscovery sourceDiscovery;

    const auto videoSources = sourceDiscovery.listDeviceCaptureSources();
    QVERIFY2(videoSources.ok, qPrintable(videoSources.message));

    const auto audioSources = sourceDiscovery.listAudioCaptureSources();
    QVERIFY2(audioSources.ok, qPrintable(audioSources.message));
}

} // namespace

QTEST_GUILESS_MAIN(SourceDiscoveryTest)

#include "source_discovery_test.moc"
