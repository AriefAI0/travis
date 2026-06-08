#include "mediaEngine/recording/recording_engine.h"

#include <chrono>

#include "mediaEngine/recording/pipeline/av_recording_pipeline.h"
#include "mediaEngine/recording/shared/recording_validation.h"

// Provides the embedded recording-engine lifecycle on top of shared source sessions.

namespace travis::media_engine::recording {

RecordingEngine::RecordingEngine(travis::media_engine::session::MediaSourceSessionManager& sessionManager)
    : sessionManager_(sessionManager) {}

RecordingEngine::~RecordingEngine() {
    (void)stopAllRecordings();
    removePreparedSource();
}

travis::media_engine::session::MediaSourceSessionResult RecordingEngine::acquireRecordingSourceSession(
    const std::string& sourceKind,
    const std::string& sourceName,
    const std::string& urlAddress,
    const std::string& devicePath,
    const std::string& sourceElement,
    travis::media_engine::session::MediaSourceSession*& session
) {
    if (sourceKind == "device-capture") {
        return sessionManager_.acquireDeviceCaptureSession(
            sourceName,
            devicePath,
            sourceElement,
            session
        );
    }

    if (sourceKind == "ndi") {
        return sessionManager_.acquireNdiSession(sourceName, urlAddress, session);
    }

    session = nullptr;
    return travis::media_engine::session::MediaSourceSessionResult{
        false,
        "Recording source is not supported yet: " + sourceKind,
    };
}

RecordingResult RecordingEngine::prepareRecordingSource(
    const std::string& sourceKind,
    const std::string& sourceName,
    const std::string& urlAddress,
    const std::string& devicePath,
    const std::string& sourceElement
) {
    if (sourceName.empty()) {
        return RecordingResult{false, "sourceName is required"};
    }

    if (preparedSource_ && preparedSource_->sourceName == sourceName) {
        return RecordingResult{true, "Recording source already prepared"};
    }

    removePreparedSource();

    travis::media_engine::session::MediaSourceSession* session = nullptr;
    const auto sessionResult = acquireRecordingSourceSession(
        sourceKind,
        sourceName,
        urlAddress,
        devicePath,
        sourceElement,
        session
    );

    if (!sessionResult.ok || session == nullptr) {
        return RecordingResult{false, sessionResult.message};
    }

    preparedSource_ = std::make_unique<PreparedRecordingSource>(PreparedRecordingSource{
        .sourceName = sourceName,
        .session = session,
    });

    return RecordingResult{true, "Recording source prepared"};
}

RecordingResult RecordingEngine::stopRecordingSource() {
    removePreparedSource();
    return RecordingResult{true, "Recording source stopped"};
}

RecordingResult RecordingEngine::startRecording(
    const std::string& recordingId,
    const std::string& sourceKind,
    const std::string& sourceName,
    const std::string& urlAddress,
    const std::string& devicePath,
    const std::string& sourceElement,
    const std::string& outputPath,
    const std::vector<RecordingVideoInput>& videoInputs,
    const std::vector<RecordingAudioInput>& audioInputs
) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    if (sourceName.empty()) {
        return RecordingResult{false, "sourceName is required"};
    }

    if (outputPath.empty()) {
        return RecordingResult{false, "outputPath is required"};
    }

    const auto outputValidationResult = validateRecordingOutputPath(outputPath);
    if (!outputValidationResult.ok) {
        return outputValidationResult;
    }

    if (videoInputs.size() != 1) {
        return RecordingResult{false, "AV recording records exactly one video source per file"};
    }

    const auto audioValidationResult = validateRecordingAudioInputs(audioInputs);
    if (!audioValidationResult.ok) {
        return audioValidationResult;
    }

    if (sourceKind != "device-capture" && sourceKind != "ndi") {
        return RecordingResult{
            false,
            "Recording source is not supported yet: " + sourceKind,
        };
    }

    if (avRecordingPipelines_.find(recordingId) != avRecordingPipelines_.end()) {
        return RecordingResult{false, "RecordingId is already active"};
    }

    if (!hasSupportedRecordingVideoEncoder()) {
        return RecordingResult{false, "No supported H.264 encoder is available for recording"};
    }

    std::vector<AvRecordingVideoInput> avVideoInputs;
    std::vector<travis::media_engine::session::MediaSourceSession*> sourceSessions;
    avVideoInputs.reserve(videoInputs.size());
    sourceSessions.reserve(videoInputs.size());

    for (const auto& videoInput : videoInputs) {
        travis::media_engine::session::MediaSourceSession* session = nullptr;
        const auto sessionResult = acquireRecordingSourceSession(
            videoInput.sourceKind,
            videoInput.sourceName,
            videoInput.urlAddress,
            videoInput.devicePath,
            videoInput.sourceElement,
            session
        );

        if (!sessionResult.ok || session == nullptr) {
            for (auto* sourceSession : sourceSessions) {
                if (sourceSession != nullptr) {
                    sessionManager_.releaseSession(*sourceSession);
                }
            }
            return RecordingResult{false, sessionResult.message};
        }

        const auto bridgeResult = sessionManager_.ensureRecordingBridge(*session);
        if (!bridgeResult.ok) {
            sessionManager_.releaseSession(*session);
            for (auto* sourceSession : sourceSessions) {
                if (sourceSession != nullptr) {
                    sessionManager_.releaseSession(*sourceSession);
                }
            }
            return RecordingResult{false, bridgeResult.message};
        }

        sourceSessions.push_back(session);
        avVideoInputs.push_back(AvRecordingVideoInput{
            .sourceKind = videoInput.sourceKind,
            .sourceName = videoInput.sourceName,
            .sourceElement = videoInput.sourceElement,
            .bridgeChannel = session->recordingBridgeChannel,
            .width = videoInput.width,
            .height = videoInput.height,
        });
    }

    auto pipeline = std::make_unique<AvRecordingPipeline>();
    const auto startResult = startAvRecordingPipeline(
        recordingId,
        avVideoInputs,
        audioInputs,
        outputPath,
        &sessionManager_,
        sourceSessions,
        *pipeline
    );

    if (!startResult.ok) {
        for (auto* sourceSession : sourceSessions) {
            if (sourceSession != nullptr) {
                sessionManager_.releaseSession(*sourceSession);
            }
        }
        return startResult;
    }

    avRecordingPipelines_[recordingId] = std::move(pipeline);
    (void)sourceKind;
    (void)sourceName;
    (void)urlAddress;
    (void)devicePath;
    (void)sourceElement;
    return startResult;
}

RecordingResult RecordingEngine::stopRecording(const std::string& recordingId) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    const auto avPipeline = avRecordingPipelines_.find(recordingId);
    if (avPipeline != avRecordingPipelines_.end()) {
        const auto finalizeResult = stopAvRecordingPipeline(*avPipeline->second);
        avRecordingPipelines_.erase(avPipeline);
        return finalizeResult;
    }

    return RecordingResult{false, "Recording is not running"};
}

RecordingResult RecordingEngine::startClipRecording(
    const std::string& recordingId,
    int clipId,
    const std::string& outputPath
) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    const auto avPipeline = avRecordingPipelines_.find(recordingId);
    if (avPipeline == avRecordingPipelines_.end()) {
        return RecordingResult{false, "Recording is not running"};
    }

    return startAvInspectionClip(*avPipeline->second, clipId, outputPath);
}

RecordingResult RecordingEngine::stopClipRecording(const std::string& recordingId, int clipId) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    const auto avPipeline = avRecordingPipelines_.find(recordingId);
    if (avPipeline == avRecordingPipelines_.end()) {
        return RecordingResult{false, "Recording is not running"};
    }

    return stopAvInspectionClip(*avPipeline->second, clipId);
}

RecordingResult RecordingEngine::cancelClipRecording(const std::string& recordingId, int clipId) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    const auto avPipeline = avRecordingPipelines_.find(recordingId);
    if (avPipeline == avRecordingPipelines_.end()) {
        return RecordingResult{false, "Recording is not running"};
    }

    return cancelAvInspectionClip(*avPipeline->second, clipId);
}

RecordingPositionResult RecordingEngine::getRecordingPosition(const std::string& recordingId) const {
    if (recordingId.empty()) {
        return RecordingPositionResult{false, "recordingId is required", 0};
    }

    const auto avPipeline = avRecordingPipelines_.find(recordingId);
    if (avPipeline != avRecordingPipelines_.end()) {
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - avPipeline->second->startedAt
                               )
                                   .count();
        return RecordingPositionResult{true, "Recording position estimated", elapsedMs};
    }

    return RecordingPositionResult{false, "Recording is not running", 0};
}

std::vector<std::string> RecordingEngine::listActiveRecordings() const {
    std::vector<std::string> recordingIds;
    recordingIds.reserve(avRecordingPipelines_.size());

    for (const auto& [recordingId, _] : avRecordingPipelines_) {
        recordingIds.push_back(recordingId);
    }

    return recordingIds;
}

RecordingResult RecordingEngine::stopAllRecordings() {
    for (const auto& [_, pipeline] : avRecordingPipelines_) {
        const auto finalizeResult = stopAvRecordingPipeline(*pipeline);
        if (!finalizeResult.ok) {
            return finalizeResult;
        }
    }

    avRecordingPipelines_.clear();
    removePreparedSource();
    return RecordingResult{true, "All recordings stopped"};
}

void RecordingEngine::removePreparedSource() {
    if (!preparedSource_) {
        return;
    }

    sessionManager_.releaseSession(*preparedSource_->session);
    preparedSource_.reset();
}

void RecordingEngine::removeAvRecordingPipelineContext(const std::string& recordingId) {
    const auto pipeline = avRecordingPipelines_.find(recordingId);

    if (pipeline == avRecordingPipelines_.end()) {
        return;
    }

    removeAvRecordingPipeline(*pipeline->second);
    avRecordingPipelines_.erase(pipeline);
}

} // namespace travis::media_engine::recording
