#pragma once

#include <QString>

#include <optional>

// Plain asset row model mapped from the SQLite asset table.

namespace travis::models {

struct Asset {
    qint64 assetId = 0;
    qint64 projectId = 0;
    QString name;
    qint64 createdAt = 0;
};

struct AssetCreateInput {
    qint64 projectId = 0;
    QString name;
};

struct AssetUpdateInput {
    std::optional<qint64> projectId;
    std::optional<QString> name;
};

} // namespace travis::models
