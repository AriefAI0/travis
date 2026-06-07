#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/master_video.h"

// Repository for CRUD access to the master_video table.

namespace travis::data::repositories {

class MasterVideoRepository : public BaseRepository {
public:
    explicit MasterVideoRepository(QSqlDatabase database);

    std::optional<travis::models::MasterVideo> create(const travis::models::MasterVideoCreateInput& input) const;
    QVector<travis::models::MasterVideo> listAll() const;
    QVector<travis::models::MasterVideo> listBySessionId(qint64 sessionId) const;
    QVector<travis::models::MasterVideo> listByStatus(const QString& status) const;
    std::optional<travis::models::MasterVideo> findById(qint64 masterVideoId) const;
    std::optional<travis::models::MasterVideo> updateById(
        qint64 masterVideoId,
        const travis::models::MasterVideoUpdateInput& input
    ) const;
    bool removeById(qint64 masterVideoId) const;
};

} // namespace travis::data::repositories
