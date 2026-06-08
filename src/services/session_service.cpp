#include "services/session_service.h"

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

} // namespace

SessionService::SessionService(
    travis::data::repositories::SessionRepository sessionRepository,
    travis::data::repositories::SessionItemRepository sessionItemRepository
)
    : sessionRepository_(std::move(sessionRepository))
    , sessionItemRepository_(std::move(sessionItemRepository)) {}

std::optional<travis::models::Session> SessionService::createSession(
    const travis::models::SessionCreateInput& input
) const {
    return sessionRepository_.create({
        .projectId = input.projectId,
        .name = normalizeOptionalText(input.name),
    });
}

QVector<travis::models::Session> SessionService::listSessions() const {
    return sessionRepository_.listAll();
}

QVector<travis::models::Session> SessionService::listSessionsByProjectId(qint64 projectId) const {
    return sessionRepository_.listByProjectId(projectId);
}

std::optional<travis::models::Session> SessionService::getSessionById(qint64 sessionId) const {
    return sessionRepository_.findById(sessionId);
}

std::optional<travis::models::Session> SessionService::getSessionByProjectIdAndName(
    qint64 projectId,
    const QString& name
) const {
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return std::nullopt;
    }

    return sessionRepository_.findByProjectIdAndName(projectId, trimmedName);
}

std::optional<travis::models::Session> SessionService::updateSession(
    qint64 sessionId,
    const travis::models::SessionUpdateInput& input
) const {
    travis::models::SessionUpdateInput normalizedInput;

    if (input.projectId.has_value()) {
        normalizedInput.projectId = input.projectId;
    }

    if (input.name.has_value()) {
        normalizedInput.name = normalizeOptionalText(*input.name);
    }

    return sessionRepository_.updateById(sessionId, normalizedInput);
}

bool SessionService::deleteSession(qint64 sessionId) const {
    return sessionRepository_.removeById(sessionId);
}

std::optional<travis::models::SessionItem> SessionService::createSessionItem(
    const travis::models::SessionItemCreateInput& input
) const {
    return sessionItemRepository_.create(input);
}

QVector<travis::models::SessionItem> SessionService::listSessionItems() const {
    return sessionItemRepository_.listAll();
}

QVector<travis::models::SessionItem> SessionService::listSessionItemsBySessionId(qint64 sessionId) const {
    return sessionItemRepository_.listBySessionId(sessionId);
}

std::optional<travis::models::SessionItem> SessionService::getSessionItemById(qint64 sessionItemId) const {
    return sessionItemRepository_.findById(sessionItemId);
}

std::optional<travis::models::SessionItem> SessionService::getSessionItemBySessionIdAndItemId(
    qint64 sessionId,
    qint64 itemId
) const {
    return sessionItemRepository_.findBySessionIdAndItemId(sessionId, itemId);
}

std::optional<travis::models::SessionItem> SessionService::updateSessionItem(
    qint64 sessionItemId,
    const travis::models::SessionItemUpdateInput& input
) const {
    return sessionItemRepository_.updateById(sessionItemId, input);
}

bool SessionService::deleteSessionItem(qint64 sessionItemId) const {
    return sessionItemRepository_.removeById(sessionItemId);
}

} // namespace travis::services
