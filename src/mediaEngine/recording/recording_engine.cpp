#include "mediaEngine/recording/recording_engine.h"

#include "mediaEngine/recording/recording_validation.h"

// Provides the first embedded recording-engine lifecycle boundary on top of shared source sessions.

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

    // The heavy AV pipeline port is intentionally added in the next slice on top of this boundary.
    (void)urlAddress;
    (void)devicePath;
    (void)sourceElement;
    (void)videoInputs;

    return RecordingResult{
        false,
        "Recording pipeline implementation is not wired yet",
    };
}

RecordingResult RecordingEngine::stopRecording(const std::string& recordingId) {
    if (recordingId.empty()) {
        return RecordingResult{false, "recordingId is required"};
    }

    return RecordingResult{false, "Recording pipeline implementation is not wired yet"};
}

RecordingPositionResult RecordingEngine::getRecordingPosition(const std::string& recordingId) const {
    if (recordingId.empty()) {
        return RecordingPositionResult{false, "recordingId is required", 0};
    }

    return RecordingPositionResult{
        false,
        "Recording pipeline implementation is not wired yet",
        0,
    };
}

std::vector<std::string> RecordingEngine::listActiveRecordings() const {
    return {};
}

RecordingResult RecordingEngine::stopAllRecordings() {
    return RecordingResult{true, "No active recordings to stop"};
}

void RecordingEngine::removePreparedSource() {
    if (!preparedSource_) {
        return;
    }

    sessionManager_.releaseSession(*preparedSource_->session);
    preparedSource_.reset();
}

} // namespace travis::media_engine::recording
