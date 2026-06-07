#pragma once

#include <optional>

// Plain session_item row model mapped from the SQLite session_item table.

namespace travis::models {

struct SessionItem {
    qint64 sessionItemId = 0;
    qint64 sessionId = 0;
    qint64 itemId = 0;
};

struct SessionItemCreateInput {
    qint64 sessionId = 0;
    qint64 itemId = 0;
};

struct SessionItemUpdateInput {
    std::optional<qint64> sessionId;
    std::optional<qint64> itemId;
};

} // namespace travis::models
