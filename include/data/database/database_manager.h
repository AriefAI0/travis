#pragma once

#include <QSqlDatabase>
#include <QString>

namespace travis::data::database {

class DatabaseManager {
public:
    DatabaseManager() = default;
    ~DatabaseManager();

    // Opens the SQLite database at the resolved application path.
    bool open();

    // Closes and unregisters the named Qt SQL connection.
    void close();

    // Returns the shared database handle for repositories.
    QSqlDatabase database() const;

    // Returns the last Qt SQL error message from open().
    QString lastError() const;

    // Returns the resolved database file path currently in use.
    QString databasePath() const;

private:
    QString databasePath_;
    QString lastError_;
};

} // namespace travis::data::database
