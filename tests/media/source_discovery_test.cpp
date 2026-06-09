#include <QtTest>

#include "mediaEngine/discovery/source_discovery.h"

namespace {

class SourceDiscoveryTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies GStreamer device discovery can run through the bundled runtime.
    void listCaptureSources_returnsStructuredResults();
    // Verifies optional NDI discovery reports structured results when the runtime exists.
    void listNdiSources_returnsStructuredResults();
};

void SourceDiscoveryTest::listCaptureSources_returnsStructuredResults() {
    travis::media_engine::discovery::SourceDiscovery sourceDiscovery;

    const auto videoSources = sourceDiscovery.listDeviceCaptureSources();
    QVERIFY2(videoSources.ok, qPrintable(videoSources.message));

    const auto audioSources = sourceDiscovery.listAudioCaptureSources();
    QVERIFY2(audioSources.ok, qPrintable(audioSources.message));
}

void SourceDiscoveryTest::listNdiSources_returnsStructuredResults() {
    travis::media_engine::discovery::SourceDiscovery sourceDiscovery;

    const auto ndiSources = sourceDiscovery.listNdiSources();
    QVERIFY2(ndiSources.ok, qPrintable(ndiSources.message));

    for (const auto& source : ndiSources.sources) {
        QCOMPARE(source.kind, QStringLiteral("ndi"));
        QVERIFY(!source.id.trimmed().isEmpty());
        QVERIFY(!source.name.trimmed().isEmpty());
    }
}

} // namespace

QTEST_GUILESS_MAIN(SourceDiscoveryTest)

#include "source_discovery_test.moc"
