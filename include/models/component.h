#pragma once

#include <QString>

#include <optional>

// Plain component row model mapped from the SQLite component table.

namespace travis::models {

struct Component {
    qint64 componentId = 0;
    qint64 assetId = 0;
    QString name;
};

struct ComponentCreateInput {
    qint64 assetId = 0;
    QString name;
};

struct ComponentUpdateInput {
    std::optional<qint64> assetId;
    std::optional<QString> name;
};

} // namespace travis::models
