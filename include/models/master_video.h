#pragma once

#include <QString>

#include <optional>

// Plain master_video row model mapped from the SQLite master_video table.

namespace travis::models {

struct MasterVideo {
    qint64 masterVideoId = 0;
    qint64 sessionId = 0;
    QString fileUrl;
    std::optional<QString> thumbnailUrl;
    qint64 startEpoch = 0;
    std::optional<qint64> endEpoch;
    QString status;
    std::optional<QString> sourceName;
};

struct MasterVideoCreateInput {
    qint64 sessionId = 0;
    QString fileUrl;
    std::optional<QString> thumbnailUrl;
    qint64 startEpoch = 0;
    std::optional<qint64> endEpoch;
    std::optional<QString> status;
    std::optional<QString> sourceName;
};

struct MasterVideoUpdateInput {
    std::optional<qint64> sessionId;
    std::optional<QString> fileUrl;
    std::optional<QString> thumbnailUrl;
    std::optional<qint64> startEpoch;
    std::optional<qint64> endEpoch;
    std::optional<QString> status;
    std::optional<QString> sourceName;
};

} // namespace travis::models
