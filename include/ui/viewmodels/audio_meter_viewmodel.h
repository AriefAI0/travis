#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include <QHash>
#include <QSet>

#include "mediaEngine/metering/audio_meter_engine.h"

// Exposes native audio meter state to QML and keeps meter lifecycle out of the AV dock.

namespace travis::ui::viewmodels {

class AudioMeterViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList levels READ levels NOTIFY levelsChanged)
    Q_PROPERTY(int revision READ revision NOTIFY levelsChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit AudioMeterViewModel(QObject* parent = nullptr);
    ~AudioMeterViewModel() override;

    [[nodiscard]] QVariantList levels() const;
    [[nodiscard]] int revision() const;
    [[nodiscard]] bool active() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool syncAudioSlots(const QVariantList& audioSlots, bool active);
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE int leftLevel(const QString& meterId) const;
    Q_INVOKABLE int rightLevel(const QString& meterId) const;

signals:
    void levelsChanged();
    void activeChanged();
    void lastErrorChanged();

private:
    struct MeterLevelState {
        QString meterId;
        int leftLevel = 0;
        int rightLevel = 0;
        double leftPeakDb = -60.0;
        double rightPeakDb = -60.0;
    };

    struct RunningMeterState {
        QString sourceKey;
        QString settingsKey;
    };

    void handleMeterLevel(const travis::media_engine::metering::AudioMeterLevel& level);
    void setActive(bool active);
    void setLastError(const QString& lastError);
    void resetLevel(const QString& meterId);
    [[nodiscard]] QString buildSourceKey(const QVariantMap& slot) const;
    [[nodiscard]] QString buildSettingsKey(const QVariantMap& slot) const;

    travis::media_engine::metering::AudioMeterEngine audioMeterEngine_;
    QHash<QString, MeterLevelState> levelsByMeterId_;
    QHash<QString, RunningMeterState> runningMeters_;
    int revision_ = 0;
    bool active_ = false;
    QString lastError_;
};

} // namespace travis::ui::viewmodels
