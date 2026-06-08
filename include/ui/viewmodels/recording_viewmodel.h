#pragma once

#include <QObject>
#include <QString>

#include <optional>
#include <vector>

#include "application/recording/recording_workflow_service.h"

// Exposes recording workflow state and clip actions to QML through a thin native viewmodel.

namespace travis::ui::viewmodels {

class RecordingViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString recordingId READ recordingId WRITE setRecordingId NOTIFY recordingIdChanged)
    Q_PROPERTY(qint64 sessionId READ sessionId WRITE setSessionId NOTIFY sessionIdChanged)
    Q_PROPERTY(QString sourceKind READ sourceKind WRITE setSourceKind NOTIFY sourceKindChanged)
    Q_PROPERTY(QString sourceName READ sourceName WRITE setSourceName NOTIFY sourceNameChanged)
    Q_PROPERTY(QString urlAddress READ urlAddress WRITE setUrlAddress NOTIFY urlAddressChanged)
    Q_PROPERTY(QString devicePath READ devicePath WRITE setDevicePath NOTIFY devicePathChanged)
    Q_PROPERTY(QString sourceElement READ sourceElement WRITE setSourceElement NOTIFY sourceElementChanged)
    Q_PROPERTY(QString sourceLabel READ sourceLabel WRITE setSourceLabel NOTIFY sourceLabelChanged)
    Q_PROPERTY(QString outputPath READ outputPath WRITE setOutputPath NOTIFY outputPathChanged)
    Q_PROPERTY(bool recordingActive READ recordingActive NOTIFY recordingActiveChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(qint64 activeMasterVideoId READ activeMasterVideoId NOTIFY activeMasterVideoChanged)
    Q_PROPERTY(QString activeMasterVideoPath READ activeMasterVideoPath NOTIFY activeMasterVideoChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit RecordingViewModel(
        travis::application::recording::RecordingWorkflowService& recordingWorkflowService,
        QObject* parent = nullptr
    );

    [[nodiscard]] QString recordingId() const;
    void setRecordingId(const QString& recordingId);

    [[nodiscard]] qint64 sessionId() const;
    void setSessionId(qint64 sessionId);

    [[nodiscard]] QString sourceKind() const;
    void setSourceKind(const QString& sourceKind);

    [[nodiscard]] QString sourceName() const;
    void setSourceName(const QString& sourceName);

    [[nodiscard]] QString urlAddress() const;
    void setUrlAddress(const QString& urlAddress);

    [[nodiscard]] QString devicePath() const;
    void setDevicePath(const QString& devicePath);

    [[nodiscard]] QString sourceElement() const;
    void setSourceElement(const QString& sourceElement);

    [[nodiscard]] QString sourceLabel() const;
    void setSourceLabel(const QString& sourceLabel);

    [[nodiscard]] QString outputPath() const;
    void setOutputPath(const QString& outputPath);

    [[nodiscard]] bool recordingActive() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] qint64 activeMasterVideoId() const;
    [[nodiscard]] QString activeMasterVideoPath() const;
    [[nodiscard]] QString statusMessage() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool startRecording();
    Q_INVOKABLE bool stopRecording();
    Q_INVOKABLE bool pauseRecording();
    Q_INVOKABLE bool resumeRecording();
    Q_INVOKABLE bool startInspectionClip(
        qint64 itemId,
        const QString& inspectionTypeName,
        const QString& clipOutputPath
    );
    Q_INVOKABLE bool stopInspectionClip(qint64 clipId);
    Q_INVOKABLE bool cancelInspectionClip(qint64 clipId);
    Q_INVOKABLE void refreshActiveRecording();

signals:
    void recordingIdChanged();
    void sessionIdChanged();
    void sourceKindChanged();
    void sourceNameChanged();
    void urlAddressChanged();
    void devicePathChanged();
    void sourceElementChanged();
    void sourceLabelChanged();
    void outputPathChanged();
    void recordingActiveChanged();
    void pausedChanged();
    void activeMasterVideoChanged();
    void statusMessageChanged();
    void lastErrorChanged();

private:
    void setStatusMessage(const QString& statusMessage);
    void setLastError(const QString& lastError);
    void syncFromActiveMasterVideo();
    void emitRecordingStateChanged();

    travis::application::recording::RecordingWorkflowService& recordingWorkflowService_;
    QString recordingId_;
    qint64 sessionId_ = 0;
    QString sourceKind_;
    QString sourceName_;
    QString urlAddress_;
    QString devicePath_;
    QString sourceElement_;
    QString sourceLabel_;
    QString outputPath_;
    bool recordingActive_ = false;
    bool paused_ = false;
    qint64 activeMasterVideoId_ = 0;
    QString activeMasterVideoPath_;
    QString statusMessage_;
    QString lastError_;
};

} // namespace travis::ui::viewmodels
