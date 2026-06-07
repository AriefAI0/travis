#pragma once

#include <QString>

#include <optional>

// Plain project row model mapped from the SQLite project table.

namespace travis::models {

struct Project {
    qint64 projectId = 0;
    QString title;
    std::optional<QString> description;
    std::optional<QString> documentId;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;
};

struct ProjectCreateInput {
    QString title;
    std::optional<QString> description;
    std::optional<QString> documentId;
};

struct ProjectUpdateInput {
    std::optional<QString> title;
    std::optional<QString> description;
    std::optional<QString> documentId;
};

} // namespace travis::models
