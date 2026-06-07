#include "data/repositories/project_repository.h"

#include <QDateTime>
#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::Project;
using travis::models::ProjectCreateInput;
using travis::models::ProjectUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

Project mapProject(const QSqlQuery& query) {
    return Project{
        .projectId = query.value("project_id").toLongLong(),
        .title = query.value("title").toString(),
        .description = optionalString(query, "description"),
        .documentId = optionalString(query, "document_id"),
        .createdAt = query.value("created_at").toLongLong(),
        .updatedAt = query.value("updated_at").toLongLong(),
    };
}

qint64 currentEpochSeconds() {
    return QDateTime::currentSecsSinceEpoch();
}

} // namespace

ProjectRepository::ProjectRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Project> ProjectRepository::create(const ProjectCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO project (title, description, document_id)
        VALUES (:title, :description, :document_id)
    )");
    query.bindValue(":title", input.title);
    query.bindValue(":description", input.description ? QVariant(*input.description) : QVariant(QVariant::String));
    query.bindValue(":document_id", input.documentId ? QVariant(*input.documentId) : QVariant(QVariant::String));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Project> ProjectRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT project_id, title, description, document_id, created_at, updated_at
        FROM project
        ORDER BY project_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Project> projects;
    while (query.next()) {
        projects.append(mapProject(query));
    }

    return projects;
}

std::optional<Project> ProjectRepository::findById(qint64 projectId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT project_id, title, description, document_id, created_at, updated_at
        FROM project
        WHERE project_id = :project_id
    )");
    query.bindValue(":project_id", projectId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapProject(query);
}

std::optional<Project> ProjectRepository::updateById(qint64 projectId, const ProjectUpdateInput& input) const {
    const std::optional<Project> existingProject = findById(projectId);
    if (!existingProject.has_value()) {
        return std::nullopt;
    }

    const Project project = *existingProject;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE project
        SET
            title = :title,
            description = :description,
            document_id = :document_id,
            updated_at = :updated_at
        WHERE project_id = :project_id
    )");
    query.bindValue(":title", input.title.value_or(project.title));
    query.bindValue(":description", input.description ? QVariant(*input.description) : (project.description ? QVariant(*project.description) : QVariant(QVariant::String)));
    query.bindValue(":document_id", input.documentId ? QVariant(*input.documentId) : (project.documentId ? QVariant(*project.documentId) : QVariant(QVariant::String)));
    query.bindValue(":updated_at", currentEpochSeconds());
    query.bindValue(":project_id", projectId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(projectId);
}

bool ProjectRepository::removeById(qint64 projectId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM project WHERE project_id = :project_id");
    query.bindValue(":project_id", projectId);

    return query.exec();
}

} // namespace travis::data::repositories
