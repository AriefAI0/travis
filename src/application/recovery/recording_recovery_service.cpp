#include "application/recovery/recording_recovery_service.h"

#include <QDateTime>
#include <QFileInfo>

// Reconciles interrupted database rows against the crash-tolerant `.mkv` files already on disk.

namespace travis::application::recovery {

RecordingRecoveryService::RecordingRecoveryService(
    travis::services::VideoService& videoService,
    travis::services::InspectionClipService& inspectionClipService,
    travis::services::ResultService& resultService
)
    : videoService_(videoService)
    , inspectionClipService_(inspectionClipService)
    , resultService_(resultService) {}

RecordingRecoveryReport RecordingRecoveryService::recoverInterruptedRecordings() const {
    RecordingRecoveryReport report;

    const auto interruptedMasterVideos = videoService_.listMasterVideosByStatus(QStringLiteral("recording"));
    for (const auto& masterVideo : interruptedMasterVideos) {
        if (fileLooksRecoverable(masterVideo.fileUrl)) {
            const auto recovered = videoService_.updateMasterVideo(
                masterVideo.masterVideoId,
                {
                    .endEpoch = QDateTime::currentSecsSinceEpoch(),
                    .status = QStringLiteral("completed"),
                }
            );

            report.recoveredMasterVideos.append({
                .before = masterVideo,
                .after = recovered,
                .action = recovered.has_value() ? QStringLiteral("completed_from_existing_file")
                                                : QStringLiteral("failed_to_complete"),
            });
        } else {
            const bool removed = videoService_.deleteMasterVideo(masterVideo.masterVideoId);
            report.recoveredMasterVideos.append({
                .before = masterVideo,
                .after = std::nullopt,
                .action = removed ? QStringLiteral("deleted_missing_or_empty_file")
                                  : QStringLiteral("failed_to_delete"),
            });
        }
    }

    const auto interruptedClips = videoService_.listVideoClipsByStatus(QStringLiteral("recording"));
    for (const auto& clip : interruptedClips) {
        if (clip.clipFileUrl.has_value() && fileLooksRecoverable(*clip.clipFileUrl)) {
            const auto recovered = videoService_.updateVideoClip(
                clip.clipId,
                {
                    .status = QStringLiteral("completed"),
                }
            );

            report.recoveredVideoClips.append({
                .before = clip,
                .after = recovered,
                .action = recovered.has_value() ? QStringLiteral("completed_from_existing_file")
                                                : QStringLiteral("failed_to_complete"),
            });
            continue;
        }

        const auto cancelled = inspectionClipService_.cancelInspectionClip(clip.clipId);
        report.recoveredVideoClips.append({
            .before = clip,
            .after = std::nullopt,
            .action = cancelled.has_value() ? QStringLiteral("cancelled_missing_or_empty_file")
                                            : QStringLiteral("failed_to_cancel"),
        });
    }

    return report;
}

bool RecordingRecoveryService::fileLooksRecoverable(const QString& path) const {
    const QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isFile() && fileInfo.size() > 0;
}

} // namespace travis::application::recovery
