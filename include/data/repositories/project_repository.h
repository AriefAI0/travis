#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/project.h"

// Repository for CRUD access to the project table.

namespace travis::data::repositories {

class ProjectRepository : public BaseRepository {
public:
    explicit ProjectRepository(QSqlDatabase database);

    std::optional<travis::models::Project> create(const travis::models::ProjectCreateInput& input) const;
    QVector<travis::models::Project> listAll() const;
    std::optional<travis::models::Project> findById(qint64 projectId) const;
    std::optional<travis::models::Project> updateById(
        qint64 projectId,
        const travis::models::ProjectUpdateInput& input
    ) const;
    bool removeById(qint64 projectId) const;
};

} // namespace travis::data::repositories
