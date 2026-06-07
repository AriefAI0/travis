#pragma once

#include <QString>

#include "data/database/database_manager.h"

// Runs database startup in the correct order: app identity first, then DB open.

namespace travis::data::database {

class DatabaseBootstrap {
public:
    // Applies app identity and opens the shared SQLite connection.
    bool initialize();

    // Returns the startup error when initialization fails.
    QString lastError() const;

    // Exposes the opened database manager for later repository use.
    DatabaseManager& databaseManager();
    const DatabaseManager& databaseManager() const;

private:
    DatabaseManager databaseManager_;
    QString lastError_;
};

} // namespace travis::data::database
