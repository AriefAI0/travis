#include "mediaEngine/recording/shared/recording_validation.h"

#include <QDir>
#include <QFileInfo>

// Enforces the currently supported audio-input constraints for recording.

namespace travis::media_engine::recording {

RecordingResult validateRecordingAudioInputs(const std::vector<RecordingAudioInput>& audioInputs) {
    if (audioInputs.size() > 4) {
        return RecordingResult{false, "Recording supports up to 4 audio inputs"};
    }

    for (const auto& audioInput : audioInputs) {
        if (audioInput.deviceName.empty()) {
            return RecordingResult{false, "Recording audio input device name is required"};
        }

        if (!audioInput.sourceElement.empty() &&
            audioInput.sourceElement != "wasapi2src" &&
            audioInput.sourceElement != "wasapisrc") {
            return RecordingResult{
                false,
                "Unsupported recording audio source element: " + audioInput.sourceElement,
            };
        }
    }

    return RecordingResult{true, "Recording audio inputs are valid"};
}

RecordingResult validateRecordingOutputPath(const std::string& outputPath) {
    if (outputPath.empty()) {
        return RecordingResult{false, "outputPath is required"};
    }

    const QFileInfo outputFile(QString::fromStdString(outputPath));
    const QDir outputDirectory = outputFile.dir();

    if (!outputDirectory.exists() && !QDir().mkpath(outputDirectory.absolutePath())) {
        return RecordingResult{
            false,
            "Failed to create recording output directory: " + outputDirectory.absolutePath().toStdString(),
        };
    }

    if (outputFile.exists() && outputFile.isDir()) {
        return RecordingResult{false, "Recording output path points to a directory"};
    }

    return RecordingResult{true, "Recording output path is valid"};
}

} // namespace travis::media_engine::recording
