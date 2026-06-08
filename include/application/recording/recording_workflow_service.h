#pragma once

#include <QObject>
#include <QHash>
#include <QString>

#include <optional>
#include <stdexcept>
#include <vector>

#include "mediaEngine/core/media_runtime.h"
#include "mediaEngine/recording/recording_engine.h"
#include "models/master_video.h"
#include "services/inspection_clip_service.h"
#include "services/video_service.h"

// Coordinates persistence services and the native recording engine for master recordings and clips.

namespace travis::application::recording {

struct StartRecordingWorkflowInput {
    QString recordingId;
    qint64 sessionId = 0;
    QString sourceKind;
    QString sourceName;
    QString urlAddress;
    QString devicePath;
    QString sourceElement;
    QString outputPath;
    QString sourceLabel;
    std::vector<travis::media_engine::recording::RecordingVideoInput> videoInputs;
    std::vector<travis::media_engine::recording::RecordingAudioInput> audioInputs;
};

struct StopRecordingWorkflowResult {
    bool ok = false;
    QString message;
    std::optional<travis::models::MasterVideo> masterVideo;
};

struct StartInspectionClipWorkflowInput {
    QString recordingId;
    qint64 itemId = 0;
    QString inspectionTypeName;
    std::optional<qint64> executionUnitId;
    std::optional<qint64> toolingId;
    std::optional<QString> value;
    std::optional<QString> remarks;
    std::optional<QString> configSnapshot;
    QString clipOutputPath;
};

struct StopInspectionClipWorkflowInput {
    QString recordingId;
    qint64 clipId = 0;
    std::optional<QString> remarks;
    std::optional<QString> thumbnailUrl;
};

class RecordingWorkflowService : public QObject {
    Q_OBJECT

public:
    RecordingWorkflowService(
        travis::services::SessionService& sessionService,
        travis::services::VideoService& videoService,
        travis::services::InspectionClipService& inspectionClipService,
        QObject* parent = nullptr
    );

    [[nodiscard]] travis::media_engine::recording::RecordingResult startRecording(
        const StartRecordingWorkflowInput& input
    );
    [[nodiscard]] StopRecordingWorkflowResult stopRecording(const QString& recordingId);
    [[nodiscard]] std::optional<travis::models::MasterVideo> getActiveMasterVideo(
        const QString& recordingId
    ) const;
    [[nodiscard]] std::optional<travis::services::InspectionClipLifecycle> startInspectionClip(
        const StartInspectionClipWorkflowInput& input
    );
    [[nodiscard]] std::optional<travis::services::InspectionClipLifecycle> stopInspectionClip(
        const StopInspectionClipWorkflowInput& input
    );
    [[nodiscard]] std::optional<travis::services::InspectionClipLifecycle> cancelInspectionClip(
        const QString& recordingId,
        qint64 clipId
    );

private:
    struct ActiveRecordingContext {
        QString recordingId;
        qint64 sessionId = 0;
        qint64 masterVideoId = 0;
        qint64 startedAtMs = 0;
        QString outputPath;
    };

    [[nodiscard]] std::optional<ActiveRecordingContext> activeRecordingContext(
        const QString& recordingId
    ) const;

    travis::services::SessionService& sessionService_;
    travis::services::VideoService& videoService_;
    travis::services::InspectionClipService& inspectionClipService_;
    travis::media_engine::core::MediaRuntime mediaRuntime_;
    travis::media_engine::session::MediaSourceSessionManager sessionManager_;
    travis::media_engine::recording::RecordingEngine recordingEngine_;
    QHash<QString, ActiveRecordingContext> activeRecordings_;
};

} // namespace travis::application::recording
