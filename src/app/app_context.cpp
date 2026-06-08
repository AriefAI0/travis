#include "app/app_context.h"

// AppContext centralizes dependency construction so startup code stays small and consistent.

namespace travis::app {

AppContext::AppContext()
    : projectRepository_(databaseBootstrap_.databaseManager().database())
    , sessionRepository_(databaseBootstrap_.databaseManager().database())
    , sessionItemRepository_(databaseBootstrap_.databaseManager().database())
    , assetRepository_(databaseBootstrap_.databaseManager().database())
    , componentRepository_(databaseBootstrap_.databaseManager().database())
    , itemRepository_(databaseBootstrap_.databaseManager().database())
    , inspectionTypeRepository_(databaseBootstrap_.databaseManager().database())
    , executionUnitRepository_(databaseBootstrap_.databaseManager().database())
    , toolingRepository_(databaseBootstrap_.databaseManager().database())
    , resultRepository_(databaseBootstrap_.databaseManager().database())
    , resultImageRepository_(databaseBootstrap_.databaseManager().database())
    , masterVideoRepository_(databaseBootstrap_.databaseManager().database())
    , videoClipRepository_(databaseBootstrap_.databaseManager().database())
    , timelineThumbnailRepository_(databaseBootstrap_.databaseManager().database())
    , projectService_(projectRepository_)
    , sessionService_(sessionRepository_, sessionItemRepository_)
    , structureService_(assetRepository_, componentRepository_, itemRepository_)
    , executionService_(executionUnitRepository_, toolingRepository_)
    , resultService_(inspectionTypeRepository_, resultRepository_, resultImageRepository_)
    , resultMediaService_(resultImageRepository_)
    , videoService_(masterVideoRepository_, videoClipRepository_, timelineThumbnailRepository_)
    , inspectionClipService_(
        resultService_,
        videoService_,
        sessionService_,
        structureService_,
        executionService_
    ) {}

bool AppContext::initialize() {
    if (!databaseBootstrap_.initialize()) {
        lastError_ = databaseBootstrap_.lastError();
        return false;
    }

    lastError_.clear();
    return true;
}

QString AppContext::lastError() const {
    return lastError_;
}

travis::services::ProjectService& AppContext::projectService() {
    return projectService_;
}

travis::services::SessionService& AppContext::sessionService() {
    return sessionService_;
}

travis::services::StructureService& AppContext::structureService() {
    return structureService_;
}

travis::services::ExecutionService& AppContext::executionService() {
    return executionService_;
}

travis::services::ResultService& AppContext::resultService() {
    return resultService_;
}

travis::services::ResultMediaService& AppContext::resultMediaService() {
    return resultMediaService_;
}

travis::services::VideoService& AppContext::videoService() {
    return videoService_;
}

travis::services::InspectionClipService& AppContext::inspectionClipService() {
    return inspectionClipService_;
}

const travis::services::ProjectService& AppContext::projectService() const {
    return projectService_;
}

const travis::services::SessionService& AppContext::sessionService() const {
    return sessionService_;
}

const travis::services::StructureService& AppContext::structureService() const {
    return structureService_;
}

const travis::services::ExecutionService& AppContext::executionService() const {
    return executionService_;
}

const travis::services::ResultService& AppContext::resultService() const {
    return resultService_;
}

const travis::services::ResultMediaService& AppContext::resultMediaService() const {
    return resultMediaService_;
}

const travis::services::VideoService& AppContext::videoService() const {
    return videoService_;
}

const travis::services::InspectionClipService& AppContext::inspectionClipService() const {
    return inspectionClipService_;
}

} // namespace travis::app
