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

    // Creates a project after validating required and optional fields.
    std::optional<travis::models::Project> createProject(const travis::models::ProjectCreateInput& input) const;
    // Returns all projects in repository order.
    QVector<travis::models::Project> listProjects() const;
    // Returns one project by id when it exists.
    std::optional<travis::models::Project> getProjectById(qint64 projectId) const;
    // Updates one project after normalizing user input.
    std::optional<travis::models::Project> updateProject(
        qint64 projectId,
        const travis::models::ProjectUpdateInput& input
    ) const;
    // Deletes one project by id.
    bool deleteProject(qint64 projectId) const;

private:
    travis::data::repositories::ProjectRepository projectRepository_;
};

} // namespace travis::services
