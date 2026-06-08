#pragma once

#include <QString>

// Builds and owns the shared database, repository, and service objects for app startup.

#include "data/database/database_bootstrap.h"
#include "data/repositories/asset_repository.h"
#include "data/repositories/component_repository.h"
#include "data/repositories/execution_unit_repository.h"
#include "data/repositories/inspection_type_repository.h"
#include "data/repositories/item_repository.h"
#include "data/repositories/master_video_repository.h"
#include "data/repositories/project_repository.h"
#include "data/repositories/result_image_repository.h"
#include "data/repositories/result_repository.h"
#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "data/repositories/timeline_thumbnail_repository.h"
#include "data/repositories/tooling_repository.h"
#include "data/repositories/video_clip_repository.h"
#include "services/execution_service.h"
#include "services/inspection_clip_service.h"
#include "services/project_service.h"
#include "services/result_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"
#include "services/video_service.h"

// Central startup container that owns the database bootstrap plus repositories and services.

namespace travis::app {

class AppContext {
public:
    AppContext();

    // Opens the database and builds all repository/service dependencies.
    bool initialize();

    QString lastError() const;

    travis::services::ProjectService& projectService();
    travis::services::SessionService& sessionService();
    travis::services::StructureService& structureService();
    travis::services::ExecutionService& executionService();
    travis::services::ResultService& resultService();
    travis::services::VideoService& videoService();
    travis::services::InspectionClipService& inspectionClipService();

    const travis::services::ProjectService& projectService() const;
    const travis::services::SessionService& sessionService() const;
    const travis::services::StructureService& structureService() const;
    const travis::services::ExecutionService& executionService() const;
    const travis::services::ResultService& resultService() const;
    const travis::services::VideoService& videoService() const;
    const travis::services::InspectionClipService& inspectionClipService() const;

private:
    travis::data::database::DatabaseBootstrap databaseBootstrap_;

    travis::data::repositories::ProjectRepository projectRepository_;
    travis::data::repositories::SessionRepository sessionRepository_;
    travis::data::repositories::SessionItemRepository sessionItemRepository_;
    travis::data::repositories::AssetRepository assetRepository_;
    travis::data::repositories::ComponentRepository componentRepository_;
    travis::data::repositories::ItemRepository itemRepository_;
    travis::data::repositories::InspectionTypeRepository inspectionTypeRepository_;
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository_;
    travis::data::repositories::ToolingRepository toolingRepository_;
    travis::data::repositories::ResultRepository resultRepository_;
    travis::data::repositories::ResultImageRepository resultImageRepository_;
    travis::data::repositories::MasterVideoRepository masterVideoRepository_;
    travis::data::repositories::VideoClipRepository videoClipRepository_;
    travis::data::repositories::TimelineThumbnailRepository timelineThumbnailRepository_;

    travis::services::ProjectService projectService_;
    travis::services::SessionService sessionService_;
    travis::services::StructureService structureService_;
    travis::services::ExecutionService executionService_;
    travis::services::ResultService resultService_;
    travis::services::VideoService videoService_;
    travis::services::InspectionClipService inspectionClipService_;

    QString lastError_;
};

} // namespace travis::app
