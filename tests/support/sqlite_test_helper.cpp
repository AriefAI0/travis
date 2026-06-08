#include "support/sqlite_test_helper.h"

#include <QSqlQuery>
#include <QStringList>

namespace travis::tests::support {

QSqlDatabase SqliteTestHelper::openInMemoryDatabase(const QString& connectionName) {
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    database.setDatabaseName(QStringLiteral(":memory:"));
    database.open();
    return database;
}

bool SqliteTestHelper::createSchema(QSqlDatabase& database) {
    QSqlQuery query(database);
    const QStringList statements = {
        QStringLiteral("PRAGMA foreign_keys = ON"),
        QStringLiteral(R"(CREATE TABLE project (
            project_id INTEGER PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT,
            document_id TEXT,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            updated_at INTEGER NOT NULL DEFAULT (strftime('%s','now'))
        ))"),
        QStringLiteral(R"(CREATE TABLE session (
            session_id INTEGER PRIMARY KEY,
            project_id INTEGER NOT NULL,
            name TEXT,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            updated_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            FOREIGN KEY (project_id) REFERENCES project(project_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE execution_unit (
            execution_unit_id INTEGER PRIMARY KEY,
            type TEXT NOT NULL,
            name TEXT NOT NULL,
            meta TEXT,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now'))
        ))"),
        QStringLiteral(R"(CREATE TABLE tooling (
            tooling_id INTEGER PRIMARY KEY,
            execution_unit_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            config TEXT,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            FOREIGN KEY (execution_unit_id) REFERENCES execution_unit(execution_unit_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE inspection_type (
            inspection_type_id INTEGER PRIMARY KEY,
            name TEXT NOT NULL UNIQUE
        ))"),
        QStringLiteral(R"(CREATE TABLE asset (
            asset_id INTEGER PRIMARY KEY,
            project_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            FOREIGN KEY (project_id) REFERENCES project(project_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE component (
            component_id INTEGER PRIMARY KEY,
            asset_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            FOREIGN KEY (asset_id) REFERENCES asset(asset_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE item (
            item_id INTEGER PRIMARY KEY,
            component_id INTEGER NOT NULL,
            item_label TEXT NOT NULL UNIQUE,
            position TEXT,
            status INTEGER,
            FOREIGN KEY (component_id) REFERENCES component(component_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE session_item (
            session_item_id INTEGER PRIMARY KEY,
            session_id INTEGER NOT NULL,
            item_id INTEGER NOT NULL,
            FOREIGN KEY (session_id) REFERENCES session(session_id) ON DELETE CASCADE,
            FOREIGN KEY (item_id) REFERENCES item(item_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE result (
            result_id INTEGER PRIMARY KEY,
            session_item_id INTEGER NOT NULL,
            inspection_type_id INTEGER NOT NULL,
            execution_unit_id INTEGER NOT NULL,
            tooling_id INTEGER,
            value TEXT,
            status TEXT,
            remarks TEXT,
            config_snapshot TEXT,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            updated_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            FOREIGN KEY (session_item_id) REFERENCES session_item(session_item_id) ON DELETE CASCADE,
            FOREIGN KEY (inspection_type_id) REFERENCES inspection_type(inspection_type_id),
            FOREIGN KEY (execution_unit_id) REFERENCES execution_unit(execution_unit_id),
            FOREIGN KEY (tooling_id) REFERENCES tooling(tooling_id)
        ))"),
        QStringLiteral(R"(CREATE TABLE master_video (
            master_video_id INTEGER PRIMARY KEY,
            session_id INTEGER NOT NULL,
            file_url TEXT NOT NULL,
            thumbnail_url TEXT,
            start_epoch INTEGER NOT NULL,
            end_epoch INTEGER,
            status TEXT NOT NULL DEFAULT 'completed'
                CHECK (status IN ('recording', 'completed', 'failed')),
            source_name TEXT,
            FOREIGN KEY (session_id) REFERENCES session(session_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE timeline_thumbnail (
            thumbnail_id INTEGER PRIMARY KEY,
            master_video_id INTEGER NOT NULL,
            timestamp_ms INTEGER NOT NULL,
            image_path TEXT NOT NULL,
            width INTEGER NOT NULL,
            height INTEGER NOT NULL,
            size_bytes INTEGER NOT NULL,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
            FOREIGN KEY (master_video_id) REFERENCES master_video(master_video_id) ON DELETE CASCADE
        ))"),
        QStringLiteral(R"(CREATE TABLE video_clip (
            clip_id INTEGER PRIMARY KEY,
            result_id INTEGER NOT NULL,
            master_video_id INTEGER NOT NULL,
            start_offset_ms INTEGER NOT NULL,
            end_offset_ms INTEGER,
            clip_file_url TEXT,
            thumbnail_url TEXT,
            status TEXT NOT NULL DEFAULT 'completed'
                CHECK (status IN ('recording', 'completed', 'failed')),
            FOREIGN KEY (result_id) REFERENCES result(result_id) ON DELETE CASCADE,
            FOREIGN KEY (master_video_id) REFERENCES master_video(master_video_id)
        ))"),
        QStringLiteral(R"(CREATE TABLE result_image (
            image_id INTEGER PRIMARY KEY,
            result_id INTEGER NOT NULL,
            raw_url TEXT NOT NULL,
            annotated_url TEXT,
            remarks TEXT,
            FOREIGN KEY (result_id) REFERENCES result(result_id) ON DELETE CASCADE
        ))")
    };

    for (const QString& statement : statements) {
        if (!query.exec(statement)) {
            return false;
        }
    }

    return true;
}

void SqliteTestHelper::closeDatabase(QSqlDatabase& database, const QString& connectionName) {
    if (database.isValid()) {
        database.close();
    }

    database = QSqlDatabase();

    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

} // namespace travis::tests::support
