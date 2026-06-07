#include "data/repositories/session_repository.h"

#include <QDateTime>
#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::Session;
using travis::models::SessionCreateInput;
using travis::models::SessionUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

Session mapSession(const QSqlQuery& query) {
    return Session{
        .sessionId = query.value("session_id").toLongLong(),
        .projectId = query.value("project_id").toLongLong(),
        .name = optionalString(query, "name"),
        .createdAt = query.value("created_at").toLongLong(),
        .updatedAt = query.value("updated_at").toLongLong(),
    };
}

qint64 currentEpochSeconds() {
    return QDateTime::currentSecsSinceEpoch();
}

} // namespace

SessionRepository::SessionRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<Session> SessionRepository::create(const SessionCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO session (project_id, name)
        VALUES (:project_id, :name)
    )");
    query.bindValue(":project_id", input.projectId);
    query.bindValue(":name", input.name ? QVariant(*input.name) : QVariant(QVariant::String));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<Session> SessionRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_id, project_id, name, created_at, updated_at
        FROM session
        ORDER BY project_id ASC, session_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<Session> sessions;
    while (query.next()) {
        sessions.append(mapSession(query));
    }

    return sessions;
}

QVector<Session> SessionRepository::listByProjectId(qint64 projectId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_id, project_id, name, created_at, updated_at
        FROM session
        WHERE project_id = :project_id
        ORDER BY session_id ASC
    )");
    query.bindValue(":project_id", projectId);

    if (!query.exec()) {
        return {};
    }

    QVector<Session> sessions;
    while (query.next()) {
        sessions.append(mapSession(query));
    }

    return sessions;
}

std::optional<Session> SessionRepository::findById(qint64 sessionId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_id, project_id, name, created_at, updated_at
        FROM session
        WHERE session_id = :session_id
    )");
    query.bindValue(":session_id", sessionId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapSession(query);
}

std::optional<Session> SessionRepository::findByProjectIdAndName(qint64 projectId, const QString& name) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_id, project_id, name, created_at, updated_at
        FROM session
        WHERE project_id = :project_id AND name = :name
    )");
    query.bindValue(":project_id", projectId);
    query.bindValue(":name", name);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapSession(query);
}

std::optional<Session> SessionRepository::updateById(qint64 sessionId, const SessionUpdateInput& input) const {
    const std::optional<Session> existingSession = findById(sessionId);
    if (!existingSession.has_value()) {
        return std::nullopt;
    }

    const Session session = *existingSession;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE session
        SET
            project_id = :project_id,
            name = :name,
            updated_at = :updated_at
        WHERE session_id = :session_id
    )");
    query.bindValue(":project_id", input.projectId.value_or(session.projectId));
    query.bindValue(":name", input.name ? QVariant(*input.name) : (session.name ? QVariant(*session.name) : QVariant(QVariant::String)));
    query.bindValue(":updated_at", currentEpochSeconds());
    query.bindValue(":session_id", sessionId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(sessionId);
}

bool SessionRepository::removeById(qint64 sessionId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM session WHERE session_id = :session_id");
    query.bindValue(":session_id", sessionId);

    return query.exec();
}

} // namespace travis::data::repositories
