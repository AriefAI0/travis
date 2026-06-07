#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/session_item.h"

// Repository for CRUD access to the session_item table.

namespace travis::data::repositories {

class SessionItemRepository : public BaseRepository {
public:
    explicit SessionItemRepository(QSqlDatabase database);

    std::optional<travis::models::SessionItem> create(const travis::models::SessionItemCreateInput& input) const;
    QVector<travis::models::SessionItem> listAll() const;
    QVector<travis::models::SessionItem> listBySessionId(qint64 sessionId) const;
    std::optional<travis::models::SessionItem> findById(qint64 sessionItemId) const;
    std::optional<travis::models::SessionItem> findBySessionIdAndItemId(qint64 sessionId, qint64 itemId) const;
    std::optional<travis::models::SessionItem> updateById(
        qint64 sessionItemId,
        const travis::models::SessionItemUpdateInput& input
    ) const;
    bool removeById(qint64 sessionItemId) const;
};

} // namespace travis::data::repositories
