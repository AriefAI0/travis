#pragma once

#include <memory>
#include <string>
#include <vector>

#include "mediaEngine/recording/shared/recording_types.h"
#include "mediaEngine/session/media_source_session.h"

// Owns the media-engine recording boundary and source-preparation lifecycle.

namespace travis::media_engine::recording {

class RecordingEngine {
public:
    explicit RecordingEngine(travis::media_engine::session::MediaSourceSessionManager& sessionManager);
    ~RecordingEngine();

    [[nodiscard]] RecordingResult prepareRecordingSource(
        const std::string& sourceKind,
        const std::string& sourceName,
        const std::string& urlAddress,
        const std::string& devicePath,
        const std::string& sourceElement
    );
    [[nodiscard]] RecordingResult stopRecordingSource();
    [[nodiscard]] RecordingResult startRecording(
        const std::string& recordingId,
        const std::string& sourceKind,
        const std::string& sourceName,
        const std::string& urlAddress,
        const std::string& devicePath,
        const std::string& sourceElement,
        const std::string& outputPath,
        const std::vector<RecordingVideoInput>& videoInputs,
        const std::vector<RecordingAudioInput>& audioInputs
    );
    [[nodiscard]] RecordingResult stopRecording(const std::string& recordingId);
    [[nodiscard]] RecordingPositionResult getRecordingPosition(const std::string& recordingId) const;
    [[nodiscard]] std::vector<std::string> listActiveRecordings() const;
    [[nodiscard]] RecordingResult stopAllRecordings();

private:
    [[nodiscard]] travis::media_engine::session::MediaSourceSessionResult acquireRecordingSourceSession(
        const std::string& sourceKind,
        const std::string& sourceName,
        const std::string& urlAddress,
        const std::string& devicePath,
        const std::string& sourceElement,
        travis::media_engine::session::MediaSourceSession*& session
    );
    void removePreparedSource();

    travis::media_engine::session::MediaSourceSessionManager& sessionManager_;
    std::unique_ptr<PreparedRecordingSource> preparedSource_;
};

} // namespace travis::media_engine::recording
