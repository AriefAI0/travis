#include "data/repositories/session_item_repository.h"

#include <QSqlQuery>

namespace travis::data::repositories {

namespace {

using travis::models::SessionItem;
using travis::models::SessionItemCreateInput;
using travis::models::SessionItemUpdateInput;

SessionItem mapSessionItem(const QSqlQuery& query) {
    return SessionItem{
        .sessionItemId = query.value("session_item_id").toLongLong(),
        .sessionId = query.value("session_id").toLongLong(),
        .itemId = query.value("item_id").toLongLong(),
    };
}

} // namespace

SessionItemRepository::SessionItemRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<SessionItem> SessionItemRepository::create(const SessionItemCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO session_item (session_id, item_id)
        VALUES (:session_id, :item_id)
    )");
    query.bindValue(":session_id", input.sessionId);
    query.bindValue(":item_id", input.itemId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<SessionItem> SessionItemRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_item_id, session_id, item_id
        FROM session_item
        ORDER BY session_id ASC, session_item_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<SessionItem> sessionItems;
    while (query.next()) {
        sessionItems.append(mapSessionItem(query));
    }

    return sessionItems;
}

QVector<SessionItem> SessionItemRepository::listBySessionId(qint64 sessionId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_item_id, session_id, item_id
        FROM session_item
        WHERE session_id = :session_id
        ORDER BY session_item_id ASC
    )");
    query.bindValue(":session_id", sessionId);

    if (!query.exec()) {
        return {};
    }

    QVector<SessionItem> sessionItems;
    while (query.next()) {
        sessionItems.append(mapSessionItem(query));
    }

    return sessionItems;
}

std::optional<SessionItem> SessionItemRepository::findById(qint64 sessionItemId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_item_id, session_id, item_id
        FROM session_item
        WHERE session_item_id = :session_item_id
    )");
    query.bindValue(":session_item_id", sessionItemId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapSessionItem(query);
}

std::optional<SessionItem> SessionItemRepository::findBySessionIdAndItemId(qint64 sessionId, qint64 itemId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT session_item_id, session_id, item_id
        FROM session_item
        WHERE session_id = :session_id AND item_id = :item_id
    )");
    query.bindValue(":session_id", sessionId);
    query.bindValue(":item_id", itemId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapSessionItem(query);
}

std::optional<SessionItem> SessionItemRepository::updateById(qint64 sessionItemId, const SessionItemUpdateInput& input) const {
    const std::optional<SessionItem> existingSessionItem = findById(sessionItemId);
    if (!existingSessionItem.has_value()) {
        return std::nullopt;
    }

    const SessionItem sessionItem = *existingSessionItem;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE session_item
        SET
            session_id = :session_id,
            item_id = :item_id
        WHERE session_item_id = :session_item_id
    )");
    query.bindValue(":session_id", input.sessionId.value_or(sessionItem.sessionId));
    query.bindValue(":item_id", input.itemId.value_or(sessionItem.itemId));
    query.bindValue(":session_item_id", sessionItemId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(sessionItemId);
}

bool SessionItemRepository::removeById(qint64 sessionItemId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM session_item WHERE session_item_id = :session_item_id");
    query.bindValue(":session_item_id", sessionItemId);

    return query.exec();
}

} // namespace travis::data::repositories
