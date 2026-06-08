#pragma once

#include <vector>

#include "mediaEngine/recording/shared/recording_types.h"

// Validates recording-input combinations before pipeline construction begins.

namespace travis::media_engine::recording {

[[nodiscard]] RecordingResult validateRecordingAudioInputs(
    const std::vector<RecordingAudioInput>& audioInputs
);

} // namespace travis::media_engine::recording
