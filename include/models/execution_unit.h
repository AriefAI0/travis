#pragma once

#include <QString>

#include <optional>

// Plain execution_unit row model mapped from the SQLite execution_unit table.

namespace travis::models {

struct ExecutionUnit {
    qint64 executionUnitId = 0;
    QString type;
    QString name;
    std::optional<QString> meta;
    qint64 createdAt = 0;
};

struct ExecutionUnitCreateInput {
    QString type;
    QString name;
    std::optional<QString> meta;
};

struct ExecutionUnitUpdateInput {
    std::optional<QString> type;
    std::optional<QString> name;
    std::optional<std::optional<QString>> meta;
};

} // namespace travis::models
