#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "application/playback/playback_workflow_service.h"

// Exposes persisted playback summaries to QML without leaking service-layer details.

namespace travis::ui::viewmodels {

class PlaybackViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(qint64 selectedMasterVideoId READ selectedMasterVideoId NOTIFY selectedMasterVideoChanged)
    Q_PROPERTY(QString selectedMasterVideoPath READ selectedMasterVideoPath NOTIFY selectedMasterVideoChanged)
    Q_PROPERTY(QStringList timelineThumbnailPaths READ timelineThumbnailPaths NOTIFY selectedMasterVideoChanged)
    Q_PROPERTY(qint64 selectedClipId READ selectedClipId NOTIFY selectedClipChanged)
    Q_PROPERTY(QString selectedClipPath READ selectedClipPath NOTIFY selectedClipChanged)
    Q_PROPERTY(QStringList resultImagePaths READ resultImagePaths NOTIFY selectedClipChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit PlaybackViewModel(
        travis::application::playback::PlaybackWorkflowService& playbackWorkflowService,
        QObject* parent = nullptr
    );

    [[nodiscard]] qint64 selectedMasterVideoId() const;
    [[nodiscard]] QString selectedMasterVideoPath() const;
    [[nodiscard]] QStringList timelineThumbnailPaths() const;
    [[nodiscard]] qint64 selectedClipId() const;
    [[nodiscard]] QString selectedClipPath() const;
    [[nodiscard]] QStringList resultImagePaths() const;
    [[nodiscard]] QString statusMessage() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool openMasterVideo(qint64 masterVideoId);
    Q_INVOKABLE bool openVideoClip(qint64 clipId);
    Q_INVOKABLE void clearSelection();

signals:
    void selectedMasterVideoChanged();
    void selectedClipChanged();
    void statusMessageChanged();
    void lastErrorChanged();

private:
    void setStatusMessage(const QString& statusMessage);
    void setLastError(const QString& lastError);

    travis::application::playback::PlaybackWorkflowService& playbackWorkflowService_;
    qint64 selectedMasterVideoId_ = 0;
    QString selectedMasterVideoPath_;
    QStringList timelineThumbnailPaths_;
    qint64 selectedClipId_ = 0;
    QString selectedClipPath_;
    QStringList resultImagePaths_;
    QString statusMessage_;
    QString lastError_;
};

} // namespace travis::ui::viewmodels
