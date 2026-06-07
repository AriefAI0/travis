#pragma once

#include <QString>

#include <QVector>

// Plain timeline_thumbnail row model mapped from the SQLite timeline_thumbnail table.

namespace travis::models {

struct TimelineThumbnail {
    qint64 thumbnailId = 0;
    qint64 masterVideoId = 0;
    qint64 timestampMs = 0;
    QString imagePath;
    qint64 width = 0;
    qint64 height = 0;
    qint64 sizeBytes = 0;
    qint64 createdAt = 0;
};

struct TimelineThumbnailCreateInput {
    qint64 masterVideoId = 0;
    qint64 timestampMs = 0;
    QString imagePath;
    qint64 width = 0;
    qint64 height = 0;
    qint64 sizeBytes = 0;
};

using TimelineThumbnailCreateBatch = QVector<TimelineThumbnailCreateInput>;

} // namespace travis::models
