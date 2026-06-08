#pragma once

#include <QString>

#include <QSqlDatabase>

// Builds disposable in-memory SQLite connections and seeds the full Travis schema for tests.

namespace travis::tests::support {

class SqliteTestHelper {
public:
    // Opens a unique in-memory SQLite connection for one test case.
    static QSqlDatabase openInMemoryDatabase(const QString& connectionName);

    // Creates the full schema used by repositories and services.
    static bool createSchema(QSqlDatabase& database);

    // Closes and unregisters a named Qt SQL connection.
    static void closeDatabase(QSqlDatabase& database, const QString& connectionName);
};

} // namespace travis::tests::support
