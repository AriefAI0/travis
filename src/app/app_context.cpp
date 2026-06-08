#include "app/app_context.h"

// AppContext centralizes dependency construction so startup code stays small and consistent.

namespace travis::app {

AppContext::AppContext() = default;

bool AppContext::initialize() {
    if (!databaseBootstrap_.initialize()) {
        lastError_ = databaseBootstrap_.lastError();
        return false;
    }

    const QSqlDatabase database = databaseBootstrap_.databaseManager().database();

    projectRepository_.emplace(database);
    sessionRepository_.emplace(database);
    sessionItemRepository_.emplace(database);
    assetRepository_.emplace(database);
    componentRepository_.emplace(database);
    itemRepository_.emplace(database);
    inspectionTypeRepository_.emplace(database);
    executionUnitRepository_.emplace(database);
    toolingRepository_.emplace(database);
    resultRepository_.emplace(database);
    resultImageRepository_.emplace(database);
    masterVideoRepository_.emplace(database);
    videoClipRepository_.emplace(database);
    timelineThumbnailRepository_.emplace(database);

    projectService_.emplace(*projectRepository_);
    sessionService_.emplace(*sessionRepository_, *sessionItemRepository_);
    structureService_.emplace(*assetRepository_, *componentRepository_, *itemRepository_);
    executionService_.emplace(*executionUnitRepository_, *toolingRepository_);
    resultService_.emplace(*inspectionTypeRepository_, *resultRepository_, *resultImageRepository_);
    resultMediaService_.emplace(*resultImageRepository_);
    videoService_.emplace(*masterVideoRepository_, *videoClipRepository_, *timelineThumbnailRepository_);
    inspectionClipService_.emplace(
        *resultService_,
        *videoService_,
        *sessionService_,
        *structureService_,
        *executionService_
    );
    recordingWorkflowService_.emplace(*sessionService_, *videoService_, *inspectionClipService_);
    playbackWorkflowService_.emplace(*videoService_, *resultMediaService_);
    recordingRecoveryService_.emplace(*videoService_, *inspectionClipService_, *resultService_);

    lastError_.clear();
    return true;
}

QString AppContext::lastError() const {
    return lastError_;
}

travis::services::ProjectService& AppContext::projectService() {
    return *projectService_;
}

travis::services::SessionService& AppContext::sessionService() {
    return *sessionService_;
}

travis::services::StructureService& AppContext::structureService() {
    return *structureService_;
}

travis::services::ExecutionService& AppContext::executionService() {
    return *executionService_;
}

travis::services::ResultService& AppContext::resultService() {
    return *resultService_;
}

travis::services::ResultMediaService& AppContext::resultMediaService() {
    return *resultMediaService_;
}

travis::services::VideoService& AppContext::videoService() {
    return *videoService_;
}

travis::services::InspectionClipService& AppContext::inspectionClipService() {
    return *inspectionClipService_;
}

travis::application::recording::RecordingWorkflowService& AppContext::recordingWorkflowService() {
    return *recordingWorkflowService_;
}

travis::application::playback::PlaybackWorkflowService& AppContext::playbackWorkflowService() {
    return *playbackWorkflowService_;
}

travis::application::recovery::RecordingRecoveryService& AppContext::recordingRecoveryService() {
    return *recordingRecoveryService_;
}

const travis::services::ProjectService& AppContext::projectService() const {
    return *projectService_;
}

const travis::services::SessionService& AppContext::sessionService() const {
    return *sessionService_;
}

const travis::services::StructureService& AppContext::structureService() const {
    return *structureService_;
}

const travis::services::ExecutionService& AppContext::executionService() const {
    return *executionService_;
}

const travis::services::ResultService& AppContext::resultService() const {
    return *resultService_;
}

const travis::services::ResultMediaService& AppContext::resultMediaService() const {
    return *resultMediaService_;
}

const travis::services::VideoService& AppContext::videoService() const {
    return *videoService_;
}

const travis::services::InspectionClipService& AppContext::inspectionClipService() const {
    return *inspectionClipService_;
}

const travis::application::recording::RecordingWorkflowService& AppContext::recordingWorkflowService() const {
    return *recordingWorkflowService_;
}

const travis::application::playback::PlaybackWorkflowService& AppContext::playbackWorkflowService() const {
    return *playbackWorkflowService_;
}

const travis::application::recovery::RecordingRecoveryService& AppContext::recordingRecoveryService() const {
    return *recordingRecoveryService_;
}

} // namespace travis::app
