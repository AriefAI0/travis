#include <QtTest>

#include "mediaEngine/metering/audio_meter_engine.h"

namespace {

class AudioMeterEngineTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies meter lifecycle validation without requiring a physical capture device.
    void lifecycleValidation_returnsStructuredResults();
};

void AudioMeterEngineTest::lifecycleValidation_returnsStructuredResults() {
    travis::media_engine::metering::AudioMeterEngine engine;

    const auto missingMeterId = engine.startMeter({
        .deviceName = "Device",
    });
    QVERIFY(!missingMeterId.ok);
    QVERIFY(!missingMeterId.message.empty());

    const auto missingDeviceName = engine.startMeter({
        .meterId = "audio-input-1",
    });
    QVERIFY(!missingDeviceName.ok);
    QVERIFY(!missingDeviceName.message.empty());

    const auto settingsForStoppedMeter = engine.setSettings("audio-input-1", 75.0, false, 0.0);
    QVERIFY2(settingsForStoppedMeter.ok, settingsForStoppedMeter.message.c_str());

    const auto stopStoppedMeter = engine.stopMeter("audio-input-1");
    QVERIFY2(stopStoppedMeter.ok, stopStoppedMeter.message.c_str());
}

} // namespace

QTEST_GUILESS_MAIN(AudioMeterEngineTest)

#include "audio_meter_engine_test.moc"
