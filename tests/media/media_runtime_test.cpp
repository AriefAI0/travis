#include <QtTest>

#include "mediaEngine/core/media_runtime.h"

namespace {

class MediaRuntimeTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies the embedded GStreamer runtime can initialize and report plugin availability.
    void healthCheck_returnsStructuredResult();
};

void MediaRuntimeTest::healthCheck_returnsStructuredResult() {
    travis::media_engine::core::MediaRuntime runtime;
    const auto healthCheck = runtime.healthCheck();

    QVERIFY(healthCheck.ok || !healthCheck.missingPlugins.isEmpty());
}

} // namespace

QTEST_GUILESS_MAIN(MediaRuntimeTest)

#include "media_runtime_test.moc"
