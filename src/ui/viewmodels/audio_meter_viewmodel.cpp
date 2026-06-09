#include "ui/viewmodels/audio_meter_viewmodel.h"

#include <QMetaObject>
#include <QVariantMap>

#include <algorithm>

// Converts native audio meter callbacks into QML-safe per-slot level state.

namespace travis::ui::viewmodels {

namespace {

int clampPercent(int value) {
    return std::max(0, std::min(100, value));
}

int clampBalance(int value) {
    return std::max(-100, std::min(100, value));
}

} // namespace

AudioMeterViewModel::AudioMeterViewModel(QObject* parent)
    : QObject(parent) {
    audioMeterEngine_.setLevelCallback([this](const auto& level) {
        QMetaObject::invokeMethod(this, [this, level]() {
            handleMeterLevel(level);
        }, Qt::QueuedConnection);
    });
}

AudioMeterViewModel::~AudioMeterViewModel() {
    audioMeterEngine_.setLevelCallback(nullptr);
    audioMeterEngine_.stopAll();
}

QVariantList AudioMeterViewModel::levels() const {
    QVariantList levelVariants;
    levelVariants.reserve(levelsByMeterId_.size());

    for (const auto& level : levelsByMeterId_) {
        levelVariants.append(QVariantMap{
            {QStringLiteral("meterId"), level.meterId},
            {QStringLiteral("leftLevel"), level.leftLevel},
            {QStringLiteral("rightLevel"), level.rightLevel},
            {QStringLiteral("leftPeakDb"), level.leftPeakDb},
            {QStringLiteral("rightPeakDb"), level.rightPeakDb},
        });
    }

    return levelVariants;
}

bool AudioMeterViewModel::active() const {
    return active_;
}

int AudioMeterViewModel::revision() const {
    return revision_;
}

QString AudioMeterViewModel::lastError() const {
    return lastError_;
}

bool AudioMeterViewModel::syncAudioSlots(const QVariantList& audioSlots, bool active) {
    setActive(active);

    if (!active) {
        stopAll();
        return true;
    }

    QSet<QString> selectedMeterIds;
    bool ok = true;

    for (const QVariant& slotValue : audioSlots) {
        const QVariantMap slot = slotValue.toMap();
        const QString meterId = slot.value(QStringLiteral("slotId")).toString().trimmed();
        const QString deviceName = slot.value(QStringLiteral("deviceName")).toString().trimmed();
        const QString devicePath = slot.value(QStringLiteral("devicePath")).toString().trimmed();
        const QString sourceElement = slot.value(QStringLiteral("sourceElement")).toString().trimmed();

        if (meterId.isEmpty()) {
            setLastError(QStringLiteral("Audio meter slotId is required"));
            ok = false;
            continue;
        }

        if (deviceName.isEmpty() && devicePath.isEmpty()) {
            const auto stopResult = audioMeterEngine_.stopMeter(meterId.toStdString());
            if (!stopResult.ok) {
                setLastError(QString::fromStdString(stopResult.message));
                ok = false;
            }
            runningMeters_.remove(meterId);
            resetLevel(meterId);
            continue;
        }

        selectedMeterIds.insert(meterId);

        const QString sourceKey = buildSourceKey(slot);
        const QString settingsKey = buildSettingsKey(slot);
        const auto runningMeter = runningMeters_.constFind(meterId);

        if (runningMeter != runningMeters_.constEnd() && runningMeter->sourceKey == sourceKey) {
            if (runningMeter->settingsKey != settingsKey) {
                const auto result = audioMeterEngine_.setSettings(
                    meterId.toStdString(),
                    clampPercent(slot.value(QStringLiteral("volume"), 100).toInt()),
                    slot.value(QStringLiteral("mono")).toBool(),
                    clampBalance(slot.value(QStringLiteral("balance")).toInt())
                );

                if (!result.ok) {
                    setLastError(QString::fromStdString(result.message));
                    ok = false;
                    continue;
                }

                runningMeters_[meterId].settingsKey = settingsKey;
            }
            continue;
        }

        if (runningMeter != runningMeters_.constEnd()) {
            const auto stopResult = audioMeterEngine_.stopMeter(meterId.toStdString());
            if (!stopResult.ok) {
                setLastError(QString::fromStdString(stopResult.message));
                ok = false;
                continue;
            }
            runningMeters_.remove(meterId);
            resetLevel(meterId);
        }

        const auto result = audioMeterEngine_.startMeter({
            .meterId = meterId.toStdString(),
            .deviceName = deviceName.toStdString(),
            .devicePath = devicePath.toStdString(),
            .sourceElement = sourceElement.toStdString(),
            .volume = static_cast<double>(clampPercent(slot.value(QStringLiteral("volume"), 100).toInt())),
            .mono = slot.value(QStringLiteral("mono")).toBool(),
            .balance = static_cast<double>(clampBalance(slot.value(QStringLiteral("balance")).toInt())),
        });

        if (!result.ok) {
            setLastError(QString::fromStdString(result.message));
            ok = false;
            continue;
        }

        runningMeters_.insert(meterId, RunningMeterState{sourceKey, settingsKey});
    }

    for (const QString& meterId : runningMeters_.keys()) {
        if (selectedMeterIds.contains(meterId)) {
            continue;
        }

        const auto stopResult = audioMeterEngine_.stopMeter(meterId.toStdString());
        if (!stopResult.ok) {
            setLastError(QString::fromStdString(stopResult.message));
            ok = false;
            continue;
        }
        runningMeters_.remove(meterId);
        resetLevel(meterId);
    }

    if (ok) {
        setLastError(QString{});
    }
    return ok;
}

void AudioMeterViewModel::stopAll() {
    audioMeterEngine_.stopAll();
    runningMeters_.clear();
    levelsByMeterId_.clear();
    ++revision_;
    emit levelsChanged();
}

int AudioMeterViewModel::leftLevel(const QString& meterId) const {
    const auto level = levelsByMeterId_.constFind(meterId);
    return level == levelsByMeterId_.constEnd() ? 0 : level->leftLevel;
}

int AudioMeterViewModel::rightLevel(const QString& meterId) const {
    const auto level = levelsByMeterId_.constFind(meterId);
    return level == levelsByMeterId_.constEnd() ? 0 : level->rightLevel;
}

void AudioMeterViewModel::handleMeterLevel(const travis::media_engine::metering::AudioMeterLevel& level) {
    const QString meterId = QString::fromStdString(level.meterId);
    levelsByMeterId_.insert(meterId, MeterLevelState{
        .meterId = meterId,
        .leftLevel = level.leftLevel,
        .rightLevel = level.rightLevel,
        .leftPeakDb = level.leftPeakDb,
        .rightPeakDb = level.rightPeakDb,
    });
    ++revision_;
    emit levelsChanged();
}

void AudioMeterViewModel::setActive(bool active) {
    if (active_ == active) {
        return;
    }

    active_ = active;
    emit activeChanged();
}

void AudioMeterViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

void AudioMeterViewModel::resetLevel(const QString& meterId) {
    if (!levelsByMeterId_.contains(meterId)) {
        return;
    }

    levelsByMeterId_.remove(meterId);
    ++revision_;
    emit levelsChanged();
}

QString AudioMeterViewModel::buildSourceKey(const QVariantMap& slot) const {
    return QStringLiteral("%1:%2:%3")
        .arg(
            slot.value(QStringLiteral("deviceName")).toString().trimmed(),
            slot.value(QStringLiteral("devicePath")).toString().trimmed(),
            slot.value(QStringLiteral("sourceElement")).toString().trimmed()
        );
}

QString AudioMeterViewModel::buildSettingsKey(const QVariantMap& slot) const {
    return QStringLiteral("%1:%2:%3")
        .arg(
            QString::number(clampPercent(slot.value(QStringLiteral("volume"), 100).toInt())),
            slot.value(QStringLiteral("mono")).toBool() ? QStringLiteral("mono") : QStringLiteral("stereo"),
            QString::number(clampBalance(slot.value(QStringLiteral("balance")).toInt()))
        );
}

} // namespace travis::ui::viewmodels
