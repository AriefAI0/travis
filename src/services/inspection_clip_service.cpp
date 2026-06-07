#include "services/inspection_clip_service.h"

#include <stdexcept>

namespace travis::services {

namespace {

void validatePositiveInteger(qint64 value, const char* fieldName) {
    if (value < 1) {
        throw std::runtime_error(fieldName);
    }
}

void validateNonNegativeInteger(qint64 value, const char* fieldName) {
    if (value < 0) {
        throw std::runtime_error(fieldName);
    }
}

QString normalizeRequiredText(const QString& value, const char* fieldName) {
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        throw std::runtime_error(fieldName);
    }

    return trimmedValue;
}

} // namespace

QString InspectionClipService::inProgressStatus() {
    return QStringLiteral("in_progress");
}

QString InspectionClipService::completedStatus() {
    return QStringLiteral("completed");
}

InspectionClipService::InspectionClipService(
    ResultService resultService,
    VideoService videoService,
    SessionService sessionService,
    StructureService structureService,
    ExecutionService executionService
)
    : resultService_(std::move(resultService))
    , videoService_(std::move(videoService))
    , sessionService_(std::move(sessionService))
    , structureService_(std::move(structureService))
    , executionService_(std::move(executionService)) {}

std::optional<InspectionClipLifecycle> InspectionClipService::getActiveInspectionClip(
    qint64 sessionItemId,
    qint64 inspectionTypeId
) const {
    validatePositiveInteger(sessionItemId, "Session item id must be positive");
    validatePositiveInteger(inspectionTypeId, "Inspection type id must be positive");

    const QVector<travis::models::Result> results = resultService_.listResultsBySessionItemId(sessionItemId);
    for (const travis::models::Result& result : results) {
        if (result.inspectionTypeId != inspectionTypeId || result.status != inProgressStatus()) {
            continue;
        }

        const std::optional<travis::models::VideoClip> clip = videoService_.getActiveVideoClipByResultId(result.resultId);
        if (!clip.has_value()) {
            throw std::runtime_error("Active result does not have an active video clip");
        }

        return InspectionClipLifecycle{
            .result = result,
            .clip = *clip,
        };
    }

    return std::nullopt;
}

InspectionClipLifecycle InspectionClipService::startInspectionClip(const StartInspectionClipInput& input) const {
    validatePositiveInteger(input.sessionItemId, "Session item id must be positive");
    validatePositiveInteger(input.inspectionTypeId, "Inspection type id must be positive");
    validatePositiveInteger(input.executionUnitId, "Execution unit id must be positive");
    validatePositiveInteger(input.masterVideoId, "Master video id must be positive");
    validateNonNegativeInteger(input.startOffsetMs, "Start offset must be non-negative");

    if (getActiveInspectionClip(input.sessionItemId, input.inspectionTypeId).has_value()) {
        throw std::runtime_error("An inspection clip is already in progress for this session item and inspection type");
    }

    const std::optional<travis::models::Result> createdResult = resultService_.createResult({
        .sessionItemId = input.sessionItemId,
        .inspectionTypeId = input.inspectionTypeId,
        .executionUnitId = input.executionUnitId,
        .toolingId = input.toolingId,
        .value = input.value,
        .status = inProgressStatus(),
        .remarks = input.remarks,
        .configSnapshot = input.configSnapshot,
    });

    if (!createdResult.has_value()) {
        throw std::runtime_error("Failed to create inspection result");
    }

    const std::optional<travis::models::VideoClip> createdClip = videoService_.startVideoClip(
        createdResult->resultId,
        input.masterVideoId,
        input.startOffsetMs
    );

    if (!createdClip.has_value()) {
        throw std::runtime_error("Failed to create inspection video clip");
    }

    return InspectionClipLifecycle{
        .result = *createdResult,
        .clip = *createdClip,
    };
}

InspectionClipLifecycle InspectionClipService::startInspectionClipFromRecording(
    const StartRecordingInspectionClipInput& input
) const {
    validatePositiveInteger(input.sessionId, "Session id must be positive");
    validatePositiveInteger(input.itemId, "Item id must be positive");
    validatePositiveInteger(input.masterVideoId, "Master video id must be positive");
    validateNonNegativeInteger(input.startOffsetMs, "Start offset must be non-negative");

    const QString inspectionTypeName = normalizeRequiredText(
        input.inspectionTypeName,
        "Inspection type name is required"
    );

    const std::optional<travis::models::InspectionType> inspectionType =
        resultService_.getInspectionTypeByName(inspectionTypeName);
    if (!inspectionType.has_value()) {
        throw std::runtime_error("Inspection type does not exist");
    }

    std::optional<travis::models::SessionItem> sessionItem =
        sessionService_.getSessionItemBySessionIdAndItemId(input.sessionId, input.itemId);

    if (!sessionItem.has_value()) {
        sessionItem = sessionService_.createSessionItem({
            .sessionId = input.sessionId,
            .itemId = input.itemId,
        });
    }

    if (!sessionItem.has_value()) {
        throw std::runtime_error("Failed to resolve session item");
    }

    qint64 executionUnitId = 0;
    if (input.executionUnitId.has_value()) {
        executionUnitId = *input.executionUnitId;
    } else {
        const QVector<travis::models::ExecutionUnit> executionUnits = executionService_.listExecutionUnits();
        if (executionUnits.size() != 1) {
            throw std::runtime_error("Execution unit id is required when there is not exactly one execution unit");
        }
        executionUnitId = executionUnits.first().executionUnitId;
    }

    return startInspectionClip({
        .sessionItemId = sessionItem->sessionItemId,
        .inspectionTypeId = inspectionType->inspectionTypeId,
        .executionUnitId = executionUnitId,
        .toolingId = input.toolingId,
        .masterVideoId = input.masterVideoId,
        .startOffsetMs = input.startOffsetMs,
        .value = input.value,
        .remarks = input.remarks,
        .configSnapshot = input.configSnapshot,
    });
}

std::optional<InspectionClipLifecycle> InspectionClipService::stopInspectionClip(
    const StopInspectionClipInput& input
) const {
    validatePositiveInteger(input.clipId, "Clip id must be positive");
    validateNonNegativeInteger(input.endOffsetMs, "End offset must be non-negative");

    const std::optional<travis::models::VideoClip> existingClip = videoService_.getVideoClipById(input.clipId);
    if (!existingClip.has_value()) {
        return std::nullopt;
    }

    if (existingClip->endOffsetMs.has_value()) {
        throw std::runtime_error("Video clip has already been completed");
    }

    if (input.endOffsetMs <= existingClip->startOffsetMs) {
        throw std::runtime_error("Video clip endOffsetMs must be greater than startOffsetMs");
    }

    const std::optional<travis::models::VideoClip> completedClip = videoService_.completeVideoClip(
        input.clipId,
        input.endOffsetMs,
        existingClip->clipFileUrl,
        input.thumbnailUrl
    );

    if (!completedClip.has_value()) {
        throw std::runtime_error("Failed to complete video clip");
    }

    const std::optional<travis::models::Result> completedResult = resultService_.updateResult(
        existingClip->resultId,
        {
            .status = completedStatus(),
            .remarks = input.remarks.has_value() ? input.remarks : std::nullopt,
        }
    );

    if (!completedResult.has_value()) {
        throw std::runtime_error("Failed to complete result");
    }

    return InspectionClipLifecycle{
        .result = *completedResult,
        .clip = *completedClip,
    };
}

std::optional<InspectionClipLifecycle> InspectionClipService::cancelInspectionClip(qint64 clipId) const {
    validatePositiveInteger(clipId, "Clip id must be positive");

    const std::optional<travis::models::VideoClip> existingClip = videoService_.getVideoClipById(clipId);
    if (!existingClip.has_value()) {
        return std::nullopt;
    }

    if (existingClip->endOffsetMs.has_value()) {
        throw std::runtime_error("Completed video clip cannot be cancelled");
    }

    const std::optional<travis::models::Result> result = resultService_.getResultById(existingClip->resultId);
    if (!result.has_value()) {
        throw std::runtime_error("Result does not exist");
    }

    if (!videoService_.deleteVideoClip(clipId)) {
        throw std::runtime_error("Failed to delete video clip");
    }

    if (!resultService_.deleteResult(existingClip->resultId)) {
        throw std::runtime_error("Failed to delete result");
    }

    return InspectionClipLifecycle{
        .result = *result,
        .clip = *existingClip,
    };
}

} // namespace travis::services
