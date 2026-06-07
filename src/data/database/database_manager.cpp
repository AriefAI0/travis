#include "data/database/database_manager.h"

#include "data/database/database_paths.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>

namespace travis::data::database {

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::open() {
    close();

    databasePath_ = DatabasePaths::resolveDatabasePath();
    const QFileInfo databaseFile(databasePath_);
    QDir().mkpath(databaseFile.dir().absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), DatabasePaths::connectionName());
    db.setDatabaseName(databasePath_);

    if (!db.open()) {
        lastError_ = db.lastError().text();
        return false;
    }

    lastError_.clear();
    return true;
}

void DatabaseManager::close() {
    const QString connectionName = DatabasePaths::connectionName();

    if (!QSqlDatabase::contains(connectionName)) {
        return;
    }

    {
        // Drop the active handle before removing the named Qt SQL connection.
        QSqlDatabase db = QSqlDatabase::database(connectionName, false);
        if (db.isValid()) {
            db.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);
}

QSqlDatabase DatabaseManager::database() const {
    return QSqlDatabase::database(DatabasePaths::connectionName(), false);
}

QString DatabaseManager::lastError() const {
    return lastError_;
}

QString DatabaseManager::databasePath() const {
    return databasePath_;
}

} // namespace travis::data::database
