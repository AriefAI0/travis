#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "models/session.h"
#include "models/session_item.h"

// Service layer for session and session-item validation plus repository orchestration.

namespace travis::services {

class SessionService {
public:
    SessionService(
        travis::data::repositories::SessionRepository sessionRepository,
        travis::data::repositories::SessionItemRepository sessionItemRepository
    );

    std::optional<travis::models::Session> createSession(const travis::models::SessionCreateInput& input) const;
    QVector<travis::models::Session> listSessions() const;
    QVector<travis::models::Session> listSessionsByProjectId(qint64 projectId) const;
    std::optional<travis::models::Session> getSessionById(qint64 sessionId) const;
    std::optional<travis::models::Session> getSessionByProjectIdAndName(qint64 projectId, const QString& name) const;
    std::optional<travis::models::Session> updateSession(
        qint64 sessionId,
        const travis::models::SessionUpdateInput& input
    ) const;
    bool deleteSession(qint64 sessionId) const;

    std::optional<travis::models::SessionItem> createSessionItem(
        const travis::models::SessionItemCreateInput& input
    ) const;
    QVector<travis::models::SessionItem> listSessionItems() const;
    QVector<travis::models::SessionItem> listSessionItemsBySessionId(qint64 sessionId) const;
    std::optional<travis::models::SessionItem> getSessionItemById(qint64 sessionItemId) const;
    std::optional<travis::models::SessionItem> getSessionItemBySessionIdAndItemId(
        qint64 sessionId,
        qint64 itemId
    ) const;
    std::optional<travis::models::SessionItem> updateSessionItem(
        qint64 sessionItemId,
        const travis::models::SessionItemUpdateInput& input
    ) const;
    bool deleteSessionItem(qint64 sessionItemId) const;

private:
    travis::data::repositories::SessionRepository sessionRepository_;
    travis::data::repositories::SessionItemRepository sessionItemRepository_;
};

} // namespace travis::services
