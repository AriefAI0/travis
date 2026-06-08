#pragma once

#include <QString>

#include <optional>

// Plain item row model mapped from the SQLite item table.

namespace travis::models {

struct Item {
    qint64 itemId = 0;
    qint64 componentId = 0;
    QString itemLabel;
    std::optional<QString> position;
    std::optional<qint64> status;
};

struct ItemCreateInput {
    qint64 componentId = 0;
    QString itemLabel;
    std::optional<QString> position;
    std::optional<qint64> status;
};

struct ItemUpdateInput {
    std::optional<qint64> componentId;
    std::optional<QString> itemLabel;
    std::optional<std::optional<QString>> position;
    std::optional<std::optional<qint64>> status;
};

} // namespace travis::models
