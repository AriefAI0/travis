#pragma once

#include <QString>

#include <optional>

#include "services/execution_service.h"
#include "services/result_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"
#include "services/video_service.h"

// Service layer for the inspection clip lifecycle across result and video services.

namespace travis::services {

struct InspectionClipLifecycle {
    travis::models::Result result;
    travis::models::VideoClip clip;
};

struct StartInspectionClipInput {
    qint64 sessionItemId = 0;
    qint64 inspectionTypeId = 0;
    qint64 executionUnitId = 0;
    std::optional<qint64> toolingId;
    qint64 masterVideoId = 0;
    qint64 startOffsetMs = 0;
    std::optional<QString> value;
    std::optional<QString> remarks;
    std::optional<QString> configSnapshot;
};

struct StartRecordingInspectionClipInput {
    qint64 sessionId = 0;
    qint64 itemId = 0;
    QString inspectionTypeName;
    std::optional<qint64> executionUnitId;
    std::optional<qint64> toolingId;
    qint64 masterVideoId = 0;
    qint64 startOffsetMs = 0;
    std::optional<QString> value;
    std::optional<QString> remarks;
    std::optional<QString> configSnapshot;
};

struct StopInspectionClipInput {
    qint64 clipId = 0;
    qint64 endOffsetMs = 0;
    std::optional<QString> thumbnailUrl;
    std::optional<QString> remarks;
};

class InspectionClipService {
public:
    static QString inProgressStatus();
    static QString completedStatus();

    InspectionClipService(
        ResultService resultService,
        VideoService videoService,
        SessionService sessionService,
        StructureService structureService,
        ExecutionService executionService
    );

    std::optional<InspectionClipLifecycle> getActiveInspectionClip(
        qint64 sessionItemId,
        qint64 inspectionTypeId
    ) const;
    InspectionClipLifecycle startInspectionClip(const StartInspectionClipInput& input) const;
    InspectionClipLifecycle startInspectionClipFromRecording(
        const StartRecordingInspectionClipInput& input
    ) const;
    std::optional<InspectionClipLifecycle> stopInspectionClip(const StopInspectionClipInput& input) const;
    std::optional<InspectionClipLifecycle> cancelInspectionClip(qint64 clipId) const;

private:
    ResultService resultService_;
    VideoService videoService_;
    SessionService sessionService_;
    StructureService structureService_;
    ExecutionService executionService_;
};

} // namespace travis::services
