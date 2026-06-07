#pragma once

#include <QString>

#include <optional>

// Plain result row model mapped from the SQLite result table.

namespace travis::models {

struct Result {
    qint64 resultId = 0;
    qint64 sessionItemId = 0;
    qint64 inspectionTypeId = 0;
    qint64 executionUnitId = 0;
    std::optional<qint64> toolingId;
    std::optional<QString> value;
    std::optional<QString> status;
    std::optional<QString> remarks;
    std::optional<QString> configSnapshot;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;
};

struct ResultCreateInput {
    qint64 sessionItemId = 0;
    qint64 inspectionTypeId = 0;
    qint64 executionUnitId = 0;
    std::optional<qint64> toolingId;
    std::optional<QString> value;
    std::optional<QString> status;
    std::optional<QString> remarks;
    std::optional<QString> configSnapshot;
};

struct ResultUpdateInput {
    std::optional<qint64> sessionItemId;
    std::optional<qint64> inspectionTypeId;
    std::optional<qint64> executionUnitId;
    std::optional<qint64> toolingId;
    std::optional<QString> value;
    std::optional<QString> status;
    std::optional<QString> remarks;
    std::optional<QString> configSnapshot;
};

} // namespace travis::models
