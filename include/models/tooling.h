#pragma once

#include <QString>

#include <optional>

// Plain tooling row model mapped from the SQLite tooling table.

namespace travis::models {

struct Tooling {
    qint64 toolingId = 0;
    qint64 executionUnitId = 0;
    QString name;
    std::optional<QString> config;
    qint64 createdAt = 0;
};

struct ToolingCreateInput {
    qint64 executionUnitId = 0;
    QString name;
    std::optional<QString> config;
};

struct ToolingUpdateInput {
    std::optional<qint64> executionUnitId;
    std::optional<QString> name;
    std::optional<QString> config;
};

} // namespace travis::models
