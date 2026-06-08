#include <QtTest>

#include <QSqlQuery>

#include "data/repositories/session_item_repository.h"
#include "data/repositories/session_repository.h"
#include "services/session_service.h"
#include "support/sqlite_test_helper.h"

namespace {

class SessionServiceTest : public QObject {
    Q_OBJECT

private slots:
    // Creates the in-memory SQLite schema used by session service tests.
    void init();
    // Closes and removes the per-test database connection.
    void cleanup();

    // Verifies session creation trims the optional name.
    void createSession_persistsNormalizedName();
    // Verifies blank session names are normalized to null on update.
    void updateSession_normalizesBlankNameToNull();
    // Verifies session-item lifecycle operations use the linked repositories correctly.
    void sessionItem_crudFlow();

private:
    QString connectionName_;
    QSqlDatabase database_;
};

void SessionServiceTest::init() {
    connectionName_ = QStringLiteral("session_service_test_connection");
    database_ = travis::tests::support::SqliteTestHelper::openInMemoryDatabase(connectionName_);
    QVERIFY2(database_.isOpen(), "Failed to open in-memory SQLite database");
    QVERIFY2(travis::tests::support::SqliteTestHelper::createSchema(database_), "Failed to create SQLite schema");

    QSqlQuery query(database_);
    QVERIFY(query.exec("INSERT INTO project (title) VALUES ('Project One')"));
    QVERIFY(query.exec("INSERT INTO asset (project_id, name) VALUES (1, 'Asset One')"));
    QVERIFY(query.exec("INSERT INTO component (asset_id, name) VALUES (1, 'Component One')"));
    QVERIFY(query.exec("INSERT INTO item (component_id, item_label) VALUES (1, 'ITEM-001')"));
}

void SessionServiceTest::cleanup() {
    travis::tests::support::SqliteTestHelper::closeDatabase(database_, connectionName_);
}

void SessionServiceTest::createSession_persistsNormalizedName() {
    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);
    travis::services::SessionService service(sessionRepository, sessionItemRepository);

    const auto createdSession = service.createSession({
        .projectId = 1,
        .name = QStringLiteral("  Dive Session  "),
    });

    QVERIFY(createdSession.has_value());
    QCOMPARE(createdSession->projectId, 1);
    QCOMPARE(createdSession->name.value(), QStringLiteral("Dive Session"));

    const QVector<travis::models::Session> sessions = service.listSessionsByProjectId(1);
    QCOMPARE(sessions.size(), 1);
    QCOMPARE(sessions.first().name.value(), QStringLiteral("Dive Session"));
}

void SessionServiceTest::updateSession_normalizesBlankNameToNull() {
    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);
    travis::services::SessionService service(sessionRepository, sessionItemRepository);

    const auto createdSession = service.createSession({
        .projectId = 1,
        .name = QStringLiteral("Session A"),
    });

    QVERIFY(createdSession.has_value());

    const auto updatedSession = service.updateSession(
        createdSession->sessionId,
        {
            .name = QStringLiteral("   "),
        }
    );

    QVERIFY(updatedSession.has_value());
    QVERIFY(!updatedSession->name.has_value());
}

void SessionServiceTest::sessionItem_crudFlow() {
    travis::data::repositories::SessionRepository sessionRepository(database_);
    travis::data::repositories::SessionItemRepository sessionItemRepository(database_);
    travis::services::SessionService service(sessionRepository, sessionItemRepository);

    const auto createdSession = service.createSession({
        .projectId = 1,
        .name = QStringLiteral("Session B"),
    });

    QVERIFY(createdSession.has_value());

    const auto createdSessionItem = service.createSessionItem({
        .sessionId = createdSession->sessionId,
        .itemId = 1,
    });

    QVERIFY(createdSessionItem.has_value());
    QCOMPARE(createdSessionItem->sessionId, createdSession->sessionId);
    QCOMPARE(createdSessionItem->itemId, 1);

    const auto foundSessionItem = service.getSessionItemBySessionIdAndItemId(createdSession->sessionId, 1);
    QVERIFY(foundSessionItem.has_value());
    QCOMPARE(foundSessionItem->sessionItemId, createdSessionItem->sessionItemId);

    const QVector<travis::models::SessionItem> sessionItems =
        service.listSessionItemsBySessionId(createdSession->sessionId);
    QCOMPARE(sessionItems.size(), 1);

    QVERIFY(service.deleteSessionItem(createdSessionItem->sessionItemId));
    QVERIFY(!service.getSessionItemById(createdSessionItem->sessionItemId).has_value());
}

} // namespace

QTEST_GUILESS_MAIN(SessionServiceTest)

#include "session_service_test.moc"
