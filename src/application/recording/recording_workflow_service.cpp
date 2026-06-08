#include "application/recording/recording_workflow_service.h"

#include <QDateTime>
// Coordinates master-video persistence and native recording-engine lifecycle in one workflow service.

namespace travis::application::recording {

namespace {

QString requiredText(const QString& value, const char* message) {
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        throw std::runtime_error(message);
    }

    return trimmedValue;
}

qint64 currentEpochSeconds() {
    return QDateTime::currentSecsSinceEpoch();
}

qint64 currentEpochMilliseconds() {
    return QDateTime::currentMSecsSinceEpoch();
}

} // namespace

RecordingWorkflowService::RecordingWorkflowService(
    travis::services::SessionService& sessionService,
    travis::services::VideoService& videoService,
    travis::services::InspectionClipService& inspectionClipService,
    QObject* parent
)
    : QObject(parent)
    , sessionService_(sessionService)
    , videoService_(videoService)
    , inspectionClipService_(inspectionClipService)
    , mediaRuntime_()
    , recordingEngine_(sessionManager_) {}

travis::media_engine::recording::RecordingResult RecordingWorkflowService::startRecording(
    const StartRecordingWorkflowInput& input
) {
    if (input.sessionId < 1) {
        return {false, "sessionId must be positive"};
    }

    const QString recordingId = requiredText(input.recordingId, "recordingId is required");
    const QString outputPath = requiredText(input.outputPath, "outputPath is required");
    const QString sourceKind = requiredText(input.sourceKind, "sourceKind is required");
    const QString sourceName = requiredText(input.sourceName, "sourceName is required");

    if (!sessionService_.getSessionById(input.sessionId).has_value()) {
        return {false, "Session does not exist"};
    }

    if (activeRecordings_.contains(recordingId)) {
        return {false, "Recording workflow is already active"};
    }

    const auto healthCheck = mediaRuntime_.healthCheck();
    if (!healthCheck.ok) {
        return {
            false,
            "Missing GStreamer plugins: " + healthCheck.missingPlugins.join(", ").toStdString(),
        };
    }

    const auto createdMasterVideo = videoService_.createMasterVideo({
        .sessionId = input.sessionId,
        .fileUrl = outputPath,
        .thumbnailUrl = std::nullopt,
        .startEpoch = currentEpochSeconds(),
        .endEpoch = std::nullopt,
        .status = QStringLiteral("recording"),
        .sourceName = input.sourceLabel.trimmed().isEmpty() ? std::optional<QString>(sourceName) : input.sourceLabel,
    });

    if (!createdMasterVideo.has_value()) {
        return {false, "Failed to create master video record"};
    }

    const qint64 startedAtMs = currentEpochMilliseconds();

    const auto startResult = recordingEngine_.startRecording(
        recordingId.toStdString(),
        sourceKind.toStdString(),
        sourceName.toStdString(),
        input.urlAddress.toStdString(),
        input.devicePath.toStdString(),
        input.sourceElement.toStdString(),
        outputPath.toStdString(),
        input.videoInputs,
        input.audioInputs
    );

    if (!startResult.ok) {
        (void)videoService_.deleteMasterVideo(createdMasterVideo->masterVideoId);
        return startResult;
    }

    activeRecordings_.insert(
        recordingId,
        ActiveRecordingContext{
            .recordingId = recordingId,
            .sessionId = input.sessionId,
            .masterVideoId = createdMasterVideo->masterVideoId,
            .startedAtMs = startedAtMs,
            .outputPath = outputPath,
        }
    );

    return startResult;
}

StopRecordingWorkflowResult RecordingWorkflowService::stopRecording(const QString& recordingId) {
    const QString normalizedRecordingId = requiredText(recordingId, "recordingId is required");
    const auto context = activeRecordingContext(normalizedRecordingId);
    if (!context.has_value()) {
        return {false, "Recording workflow is not active", std::nullopt};
    }

    const auto stopResult = recordingEngine_.stopRecording(normalizedRecordingId.toStdString());
    if (!stopResult.ok) {
        return {false, QString::fromStdString(stopResult.message), std::nullopt};
    }

    const auto updatedMasterVideo = videoService_.updateMasterVideo(
        context->masterVideoId,
        {
            .endEpoch = currentEpochSeconds(),
            .status = QStringLiteral("completed"),
        }
    );

    if (!updatedMasterVideo.has_value()) {
        return {false, "Recording stopped but failed to update master video state", std::nullopt};
    }

    activeRecordings_.remove(normalizedRecordingId);
    return {true, QString::fromStdString(stopResult.message), updatedMasterVideo};
}

std::optional<travis::models::MasterVideo> RecordingWorkflowService::getActiveMasterVideo(
    const QString& recordingId
) const {
    const auto context = activeRecordingContext(recordingId);
    if (!context.has_value()) {
        return std::nullopt;
    }

    return videoService_.getMasterVideoById(context->masterVideoId);
}

std::optional<travis::services::InspectionClipLifecycle> RecordingWorkflowService::startInspectionClip(
    const StartInspectionClipWorkflowInput& input
) {
    const auto context = activeRecordingContext(input.recordingId);
    if (!context.has_value()) {
        throw std::runtime_error("Recording workflow is not active");
    }

    const auto clipLifecycle = inspectionClipService_.startInspectionClipFromRecording({
        .sessionId = context->sessionId,
        .itemId = input.itemId,
        .inspectionTypeName = input.inspectionTypeName,
        .executionUnitId = input.executionUnitId,
        .toolingId = input.toolingId,
        .masterVideoId = context->masterVideoId,
        .startOffsetMs = currentEpochMilliseconds() - context->startedAtMs,
        .value = input.value,
        .remarks = input.remarks,
        .configSnapshot = input.configSnapshot,
    });

    const auto engineResult = recordingEngine_.startClipRecording(
        input.recordingId.toStdString(),
        static_cast<int>(clipLifecycle.clip.clipId),
        requiredText(input.clipOutputPath, "clipOutputPath is required").toStdString()
    );

    if (!engineResult.ok) {
        (void)inspectionClipService_.cancelInspectionClip(clipLifecycle.clip.clipId);
        throw std::runtime_error(engineResult.message);
    }

    const auto updatedClip = videoService_.updateVideoClip(
        clipLifecycle.clip.clipId,
        {
            .clipFileUrl = input.clipOutputPath,
        }
    );

    if (!updatedClip.has_value()) {
        throw std::runtime_error("Clip started but failed to persist clip file path");
    }

    return travis::services::InspectionClipLifecycle{
        .result = clipLifecycle.result,
        .clip = *updatedClip,
    };
}

std::optional<travis::services::InspectionClipLifecycle> RecordingWorkflowService::stopInspectionClip(
    const StopInspectionClipWorkflowInput& input
) {
    const auto context = activeRecordingContext(input.recordingId);
    if (!context.has_value()) {
        throw std::runtime_error("Recording workflow is not active");
    }

    const auto existingClip = videoService_.getVideoClipById(input.clipId);
    if (!existingClip.has_value()) {
        return std::nullopt;
    }

    const auto engineResult = recordingEngine_.stopClipRecording(
        input.recordingId.toStdString(),
        static_cast<int>(input.clipId)
    );

    if (!engineResult.ok) {
        throw std::runtime_error(engineResult.message);
    }

    const qint64 endOffsetMs = currentEpochMilliseconds() - context->startedAtMs;
    return inspectionClipService_.stopInspectionClip({
        .clipId = input.clipId,
        .endOffsetMs = endOffsetMs,
        .thumbnailUrl = input.thumbnailUrl,
        .remarks = input.remarks,
    });
}

std::optional<travis::services::InspectionClipLifecycle> RecordingWorkflowService::cancelInspectionClip(
    const QString& recordingId,
    qint64 clipId
) {
    const auto context = activeRecordingContext(recordingId);
    if (!context.has_value()) {
        throw std::runtime_error("Recording workflow is not active");
    }

    const auto engineResult = recordingEngine_.cancelClipRecording(
        recordingId.toStdString(),
        static_cast<int>(clipId)
    );

    if (!engineResult.ok) {
        throw std::runtime_error(engineResult.message);
    }

    return inspectionClipService_.cancelInspectionClip(clipId);
}

std::optional<RecordingWorkflowService::ActiveRecordingContext> RecordingWorkflowService::activeRecordingContext(
    const QString& recordingId
) const {
    const QString normalizedRecordingId = recordingId.trimmed();
    const auto activeRecording = activeRecordings_.find(normalizedRecordingId);
    if (activeRecording == activeRecordings_.end()) {
        return std::nullopt;
    }

    return *activeRecording;
}

} // namespace travis::application::recording
