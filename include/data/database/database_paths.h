#pragma once

#include <QString>

// Resolves where the SQLite database file should live in development and production.

namespace travis::data::database {

class DatabasePaths {
public:
    // Shared Qt SQL connection identifier used across the app.
    static QString connectionName();

    // Repo-local database path for development runs.
    static QString developmentDatabasePath();

    // Writable per-user database path for installed production builds.
    static QString productionDatabasePath();

    // Chooses development or production path based on the current runtime location.
    static QString resolveDatabasePath();
};

} // namespace travis::data::database
