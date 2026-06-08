#include "services/project_service.h"

#include <stdexcept>

namespace travis::services {

namespace {

std::optional<QString> normalizeOptionalText(const std::optional<QString>& value) {
    if (!value.has_value()) {
        return std::nullopt;
    }

    const QString trimmedValue = value->trimmed();
    if (trimmedValue.isEmpty()) {
        return std::nullopt;
    }

    return trimmedValue;
}

QString normalizeRequiredTitle(const QString& value) {
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        throw std::runtime_error("Project title is required");
    }

    return trimmedValue;
}

} // namespace

ProjectService::ProjectService(travis::data::repositories::ProjectRepository projectRepository)
    : projectRepository_(std::move(projectRepository)) {}

std::optional<travis::models::Project> ProjectService::createProject(
    const travis::models::ProjectCreateInput& input
) const {
    return projectRepository_.create({
        .title = normalizeRequiredTitle(input.title),
        .description = normalizeOptionalText(input.description),
        .documentId = normalizeOptionalText(input.documentId),
    });
}

QVector<travis::models::Project> ProjectService::listProjects() const {
    return projectRepository_.listAll();
}

std::optional<travis::models::Project> ProjectService::getProjectById(qint64 projectId) const {
    return projectRepository_.findById(projectId);
}

std::optional<travis::models::Project> ProjectService::updateProject(
    qint64 projectId,
    const travis::models::ProjectUpdateInput& input
) const {
    travis::models::ProjectUpdateInput normalizedInput;

    if (input.title.has_value()) {
        normalizedInput.title = normalizeRequiredTitle(*input.title);
    }

    if (input.description.has_value()) {
        normalizedInput.description = normalizeOptionalText(*input.description);
    }

    if (input.documentId.has_value()) {
        normalizedInput.documentId = normalizeOptionalText(*input.documentId);
    }

    return projectRepository_.updateById(projectId, normalizedInput);
}

bool ProjectService::deleteProject(qint64 projectId) const {
    return projectRepository_.removeById(projectId);
}

} // namespace travis::services
