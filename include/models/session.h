#pragma once

#include <QString>

#include <optional>

// Plain session row model mapped from the SQLite session table.

namespace travis::models {

struct Session {
    qint64 sessionId = 0;
    qint64 projectId = 0;
    std::optional<QString> name;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;
};

struct SessionCreateInput {
    qint64 projectId = 0;
    std::optional<QString> name;
};

struct SessionUpdateInput {
    std::optional<qint64> projectId;
    std::optional<QString> name;
};

} // namespace travis::models
