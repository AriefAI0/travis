#pragma once

#include <QVector>

#include "services/inspection_clip_service.h"
#include "services/result_service.h"
#include "services/video_service.h"

// Reconciles interrupted recording and clip rows against the persisted `.mkv` file state.

namespace travis::application::recovery {

struct RecoveredMasterVideo {
    travis::models::MasterVideo before;
    std::optional<travis::models::MasterVideo> after;
    QString action;
};

struct RecoveredVideoClip {
    travis::models::VideoClip before;
    std::optional<travis::models::VideoClip> after;
    QString action;
};

struct RecordingRecoveryReport {
    QVector<RecoveredMasterVideo> recoveredMasterVideos;
    QVector<RecoveredVideoClip> recoveredVideoClips;
};

class RecordingRecoveryService {
public:
    RecordingRecoveryService(
        travis::services::VideoService& videoService,
        travis::services::InspectionClipService& inspectionClipService,
        travis::services::ResultService& resultService
    );

    [[nodiscard]] RecordingRecoveryReport recoverInterruptedRecordings() const;

private:
    [[nodiscard]] bool fileLooksRecoverable(const QString& path) const;

    travis::services::VideoService& videoService_;
    travis::services::InspectionClipService& inspectionClipService_;
    travis::services::ResultService& resultService_;
};

} // namespace travis::application::recovery
