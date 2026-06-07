#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/session.h"

// Repository for CRUD access to the session table.

namespace travis::data::repositories {

class SessionRepository : public BaseRepository {
public:
    explicit SessionRepository(QSqlDatabase database);

    std::optional<travis::models::Session> create(const travis::models::SessionCreateInput& input) const;
    QVector<travis::models::Session> listAll() const;
    QVector<travis::models::Session> listByProjectId(qint64 projectId) const;
    std::optional<travis::models::Session> findById(qint64 sessionId) const;
    std::optional<travis::models::Session> findByProjectIdAndName(qint64 projectId, const QString& name) const;
    std::optional<travis::models::Session> updateById(
        qint64 sessionId,
        const travis::models::SessionUpdateInput& input
    ) const;
    bool removeById(qint64 sessionId) const;
};

} // namespace travis::data::repositories
