#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/project_repository.h"
#include "models/project.h"

// Service layer for project validation and project repository orchestration.

namespace travis::services {

class ProjectService {
public:
    explicit ProjectService(travis::data::repositories::ProjectRepository projectRepository);

    std::optional<travis::models::Project> createProject(const travis::models::ProjectCreateInput& input) const;
    QVector<travis::models::Project> listProjects() const;
    std::optional<travis::models::Project> getProjectById(qint64 projectId) const;
    std::optional<travis::models::Project> updateProject(
        qint64 projectId,
        const travis::models::ProjectUpdateInput& input
    ) const;
    bool deleteProject(qint64 projectId) const;

private:
    travis::data::repositories::ProjectRepository projectRepository_;
};

} // namespace travis::services
