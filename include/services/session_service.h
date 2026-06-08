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

    // Creates a session after normalizing optional fields.
    std::optional<travis::models::Session> createSession(const travis::models::SessionCreateInput& input) const;
    // Returns all sessions.
    QVector<travis::models::Session> listSessions() const;
    // Returns sessions for one project.
    QVector<travis::models::Session> listSessionsByProjectId(qint64 projectId) const;
    // Returns one session by id when it exists.
    std::optional<travis::models::Session> getSessionById(qint64 sessionId) const;
    // Returns one session matching project id and session name.
    std::optional<travis::models::Session> getSessionByProjectIdAndName(qint64 projectId, const QString& name) const;
    // Updates one session after normalizing optional fields.
    std::optional<travis::models::Session> updateSession(
        qint64 sessionId,
        const travis::models::SessionUpdateInput& input
    ) const;
    // Deletes one session by id.
    bool deleteSession(qint64 sessionId) const;

    // Creates a session-item link row.
    std::optional<travis::models::SessionItem> createSessionItem(
        const travis::models::SessionItemCreateInput& input
    ) const;
    // Returns all session-item link rows.
    QVector<travis::models::SessionItem> listSessionItems() const;
    // Returns session-item link rows for one session.
    QVector<travis::models::SessionItem> listSessionItemsBySessionId(qint64 sessionId) const;
    // Returns one session-item link row by id.
    std::optional<travis::models::SessionItem> getSessionItemById(qint64 sessionItemId) const;
    // Returns one session-item link row by session and item.
    std::optional<travis::models::SessionItem> getSessionItemBySessionIdAndItemId(
        qint64 sessionId,
        qint64 itemId
    ) const;
    // Updates one session-item link row.
    std::optional<travis::models::SessionItem> updateSessionItem(
        qint64 sessionItemId,
        const travis::models::SessionItemUpdateInput& input
    ) const;
    // Deletes one session-item link row by id.
    bool deleteSessionItem(qint64 sessionItemId) const;

private:
    travis::data::repositories::SessionRepository sessionRepository_;
    travis::data::repositories::SessionItemRepository sessionItemRepository_;
};

} // namespace travis::services
