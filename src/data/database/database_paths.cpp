#include "data/database/database_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace travis::data::database {

namespace {

QString cleanPath(const QString& path) {
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

bool useDevelopmentDatabase() {
    // When running from the build tree, keep using the repo-local database file.
    const QString appDir = QCoreApplication::applicationDirPath();
    return QFileInfo::exists(QDir(appDir).filePath("../../database"));
}

} // namespace

QString DatabasePaths::connectionName() {
    return QStringLiteral("travis.sqlite.connection");
}

QString DatabasePaths::developmentDatabasePath() {
    const QString appDir = QCoreApplication::applicationDirPath();
    return cleanPath(QDir(appDir).filePath("../../database/travis.db"));
}

QString DatabasePaths::productionDatabasePath() {
    // Qt derives AppDataLocation from the configured organization and application names.
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return cleanPath(QDir(appDataDir).filePath("database/travis.db"));
}

QString DatabasePaths::resolveDatabasePath() {
    if (useDevelopmentDatabase()) {
        return developmentDatabasePath();
    }

    return productionDatabasePath();
}

} // namespace travis::data::database
