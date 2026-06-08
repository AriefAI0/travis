#include "ui/viewmodels/recording_viewmodel.h"

#include <QVariantMap>

// Bridges QML recording actions to the application workflow while keeping UI state explicit.

namespace travis::ui::viewmodels {

RecordingViewModel::RecordingViewModel(
    travis::application::recording::RecordingWorkflowService& recordingWorkflowService,
    QObject* parent
)
    : QObject(parent)
    , recordingWorkflowService_(recordingWorkflowService) {}

QString RecordingViewModel::recordingId() const {
    return recordingId_;
}

void RecordingViewModel::setRecordingId(const QString& recordingId) {
    if (recordingId_ == recordingId) {
        return;
    }

    recordingId_ = recordingId;
    emit recordingIdChanged();
}

qint64 RecordingViewModel::sessionId() const {
    return sessionId_;
}

void RecordingViewModel::setSessionId(qint64 sessionId) {
    if (sessionId_ == sessionId) {
        return;
    }

    sessionId_ = sessionId;
    emit sessionIdChanged();
}

QString RecordingViewModel::sourceKind() const {
    return sourceKind_;
}

void RecordingViewModel::setSourceKind(const QString& sourceKind) {
    if (sourceKind_ == sourceKind) {
        return;
    }

    sourceKind_ = sourceKind;
    emit sourceKindChanged();
}

QString RecordingViewModel::sourceName() const {
    return sourceName_;
}

void RecordingViewModel::setSourceName(const QString& sourceName) {
    if (sourceName_ == sourceName) {
        return;
    }

    sourceName_ = sourceName;
    emit sourceNameChanged();
}

QString RecordingViewModel::urlAddress() const {
    return urlAddress_;
}

void RecordingViewModel::setUrlAddress(const QString& urlAddress) {
    if (urlAddress_ == urlAddress) {
        return;
    }

    urlAddress_ = urlAddress;
    emit urlAddressChanged();
}

QString RecordingViewModel::devicePath() const {
    return devicePath_;
}

void RecordingViewModel::setDevicePath(const QString& devicePath) {
    if (devicePath_ == devicePath) {
        return;
    }

    devicePath_ = devicePath;
    emit devicePathChanged();
}

QString RecordingViewModel::sourceElement() const {
    return sourceElement_;
}

void RecordingViewModel::setSourceElement(const QString& sourceElement) {
    if (sourceElement_ == sourceElement) {
        return;
    }

    sourceElement_ = sourceElement;
    emit sourceElementChanged();
}

QString RecordingViewModel::sourceLabel() const {
    return sourceLabel_;
}

void RecordingViewModel::setSourceLabel(const QString& sourceLabel) {
    if (sourceLabel_ == sourceLabel) {
        return;
    }

    sourceLabel_ = sourceLabel;
    emit sourceLabelChanged();
}

QString RecordingViewModel::outputPath() const {
    return outputPath_;
}

void RecordingViewModel::setOutputPath(const QString& outputPath) {
    if (outputPath_ == outputPath) {
        return;
    }

    outputPath_ = outputPath;
    emit outputPathChanged();
}

bool RecordingViewModel::recordingActive() const {
    return recordingActive_;
}

bool RecordingViewModel::paused() const {
    return paused_;
}

qint64 RecordingViewModel::activeMasterVideoId() const {
    return activeMasterVideoId_;
}

QString RecordingViewModel::activeMasterVideoPath() const {
    return activeMasterVideoPath_;
}

QString RecordingViewModel::statusMessage() const {
    return statusMessage_;
}

QString RecordingViewModel::lastError() const {
    return lastError_;
}

bool RecordingViewModel::configureAudioInputs(const QVariantList& audioInputs) {
    std::vector<travis::media_engine::recording::RecordingAudioInput> nextAudioInputs;
    nextAudioInputs.reserve(static_cast<std::size_t>(audioInputs.size()));

    for (const QVariant& audioInputValue : audioInputs) {
        const QVariantMap audioInput = audioInputValue.toMap();
        const QString deviceName = audioInput.value(QStringLiteral("deviceName")).toString().trimmed();
        const QString devicePath = audioInput.value(QStringLiteral("devicePath")).toString().trimmed();
        const QString sourceElement = audioInput.value(QStringLiteral("sourceElement")).toString().trimmed();

        if (deviceName.isEmpty() && devicePath.isEmpty()) {
            setLastError(QStringLiteral("Each audio input needs a device name or device path"));
            return false;
        }

        nextAudioInputs.push_back(travis::media_engine::recording::RecordingAudioInput{
            .deviceName = deviceName.toStdString(),
            .devicePath = devicePath.toStdString(),
            .sourceElement = sourceElement.toStdString(),
        });
    }

    audioInputs_ = std::move(nextAudioInputs);
    setLastError(QString{});
    setStatusMessage(QStringLiteral("Audio inputs updated"));
    return true;
}

bool RecordingViewModel::startRecording() {
    if (sourceKind_.trimmed().isEmpty()) {
        setLastError(QStringLiteral("sourceKind is required"));
        return false;
    }

    if (sourceName_.trimmed().isEmpty()) {
        setLastError(QStringLiteral("sourceName is required"));
        return false;
    }

    std::vector<travis::media_engine::recording::RecordingVideoInput> videoInputs;
    videoInputs.push_back(travis::media_engine::recording::RecordingVideoInput{
        .sourceKind = sourceKind_.trimmed().toStdString(),
        .sourceName = sourceName_.trimmed().toStdString(),
        .urlAddress = urlAddress_.trimmed().toStdString(),
        .devicePath = devicePath_.trimmed().toStdString(),
        .sourceElement = sourceElement_.trimmed().toStdString(),
        .width = 1920,
        .height = 1080,
    });

    const auto result = recordingWorkflowService_.startRecording({
        .recordingId = recordingId_,
        .sessionId = sessionId_,
        .sourceKind = sourceKind_,
        .sourceName = sourceName_,
        .urlAddress = urlAddress_,
        .devicePath = devicePath_,
        .sourceElement = sourceElement_,
        .outputPath = outputPath_,
        .sourceLabel = sourceLabel_,
        .videoInputs = videoInputs,
        .audioInputs = audioInputs_,
    });

    if (!result.ok) {
        setLastError(QString::fromStdString(result.message));
        return false;
    }

    recordingActive_ = true;
    paused_ = false;
    syncFromActiveMasterVideo();
    emitRecordingStateChanged();
    setLastError(QString{});
    setStatusMessage(QString::fromStdString(result.message));
    return true;
}

bool RecordingViewModel::stopRecording() {
    const auto result = recordingWorkflowService_.stopRecording(recordingId_);
    if (!result.ok) {
        setLastError(result.message);
        return false;
    }

    recordingActive_ = false;
    paused_ = false;
    activeMasterVideoId_ = result.masterVideo.has_value() ? result.masterVideo->masterVideoId : 0;
    activeMasterVideoPath_ = result.masterVideo.has_value() ? result.masterVideo->fileUrl : QString{};
    emitRecordingStateChanged();
    emit activeMasterVideoChanged();
    setLastError(QString{});
    setStatusMessage(result.message);
    return true;
}

bool RecordingViewModel::pauseRecording() {
    const auto result = recordingWorkflowService_.pauseRecording(recordingId_);
    if (!result.ok) {
        setLastError(result.message);
        return false;
    }

    paused_ = true;
    emit pausedChanged();
    setLastError(QString{});
    setStatusMessage(result.message);
    return true;
}

bool RecordingViewModel::resumeRecording() {
    const auto result = recordingWorkflowService_.resumeRecording(recordingId_);
    if (!result.ok) {
        setLastError(result.message);
        return false;
    }

    paused_ = false;
    emit pausedChanged();
    setLastError(QString{});
    setStatusMessage(result.message);
    return true;
}

bool RecordingViewModel::startInspectionClip(
    qint64 itemId,
    const QString& inspectionTypeName,
    const QString& clipOutputPath
) {
    try {
        const auto clipLifecycle = recordingWorkflowService_.startInspectionClip({
            .recordingId = recordingId_,
            .itemId = itemId,
            .inspectionTypeName = inspectionTypeName,
            .clipOutputPath = clipOutputPath,
        });

        if (!clipLifecycle.has_value()) {
            setLastError(QStringLiteral("Failed to start inspection clip"));
            return false;
        }

        setLastError(QString{});
        setStatusMessage(QStringLiteral("Inspection clip started"));
        return true;
    } catch (const std::exception& exception) {
        setLastError(QString::fromUtf8(exception.what()));
        return false;
    }
}

bool RecordingViewModel::stopInspectionClip(qint64 clipId) {
    try {
        const auto clipLifecycle = recordingWorkflowService_.stopInspectionClip({
            .recordingId = recordingId_,
            .clipId = clipId,
        });

        if (!clipLifecycle.has_value()) {
            setLastError(QStringLiteral("Inspection clip not found"));
            return false;
        }

        setLastError(QString{});
        setStatusMessage(QStringLiteral("Inspection clip stopped"));
        return true;
    } catch (const std::exception& exception) {
        setLastError(QString::fromUtf8(exception.what()));
        return false;
    }
}

bool RecordingViewModel::cancelInspectionClip(qint64 clipId) {
    try {
        const auto clipLifecycle = recordingWorkflowService_.cancelInspectionClip(recordingId_, clipId);
        if (!clipLifecycle.has_value()) {
            setLastError(QStringLiteral("Inspection clip not found"));
            return false;
        }

        setLastError(QString{});
        setStatusMessage(QStringLiteral("Inspection clip cancelled"));
        return true;
    } catch (const std::exception& exception) {
        setLastError(QString::fromUtf8(exception.what()));
        return false;
    }
}

void RecordingViewModel::refreshActiveRecording() {
    syncFromActiveMasterVideo();
}

void RecordingViewModel::setStatusMessage(const QString& statusMessage) {
    if (statusMessage_ == statusMessage) {
        return;
    }

    statusMessage_ = statusMessage;
    emit statusMessageChanged();
}

void RecordingViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

void RecordingViewModel::syncFromActiveMasterVideo() {
    const auto activeMasterVideo = recordingWorkflowService_.getActiveMasterVideo(recordingId_);
    recordingActive_ = activeMasterVideo.has_value();
    activeMasterVideoId_ = activeMasterVideo.has_value() ? activeMasterVideo->masterVideoId : 0;
    activeMasterVideoPath_ = activeMasterVideo.has_value() ? activeMasterVideo->fileUrl : QString{};
    emitRecordingStateChanged();
    emit activeMasterVideoChanged();
}

void RecordingViewModel::emitRecordingStateChanged() {
    emit recordingActiveChanged();
    emit pausedChanged();
}

} // namespace travis::ui::viewmodels
