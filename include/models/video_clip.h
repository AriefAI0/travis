#pragma once

#include <QString>

#include <optional>

// Plain video_clip row model mapped from the SQLite video_clip table.

namespace travis::models {

struct VideoClip {
    qint64 clipId = 0;
    qint64 resultId = 0;
    qint64 masterVideoId = 0;
    qint64 startOffsetMs = 0;
    std::optional<qint64> endOffsetMs;
    std::optional<QString> clipFileUrl;
    std::optional<QString> thumbnailUrl;
    QString status;
};

struct VideoClipCreateInput {
    qint64 resultId = 0;
    qint64 masterVideoId = 0;
    qint64 startOffsetMs = 0;
    std::optional<qint64> endOffsetMs;
    std::optional<QString> clipFileUrl;
    std::optional<QString> thumbnailUrl;
    std::optional<QString> status;
};

struct VideoClipUpdateInput {
    std::optional<qint64> resultId;
    std::optional<qint64> masterVideoId;
    std::optional<qint64> startOffsetMs;
    std::optional<qint64> endOffsetMs;
    std::optional<QString> clipFileUrl;
    std::optional<QString> thumbnailUrl;
    std::optional<QString> status;
};

} // namespace travis::models
