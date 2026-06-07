#include "data/repositories/timeline_thumbnail_repository.h"

#include <QSqlQuery>

namespace travis::data::repositories {

namespace {

using travis::models::TimelineThumbnail;
using travis::models::TimelineThumbnailCreateBatch;
using travis::models::TimelineThumbnailCreateInput;

TimelineThumbnail mapTimelineThumbnail(const QSqlQuery& query) {
    return TimelineThumbnail{
        .thumbnailId = query.value("thumbnail_id").toLongLong(),
        .masterVideoId = query.value("master_video_id").toLongLong(),
        .timestampMs = query.value("timestamp_ms").toLongLong(),
        .imagePath = query.value("image_path").toString(),
        .width = query.value("width").toLongLong(),
        .height = query.value("height").toLongLong(),
        .sizeBytes = query.value("size_bytes").toLongLong(),
        .createdAt = query.value("created_at").toLongLong(),
    };
}

} // namespace

TimelineThumbnailRepository::TimelineThumbnailRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

QVector<TimelineThumbnail> TimelineThumbnailRepository::createMany(const TimelineThumbnailCreateBatch& input) const {
    if (input.isEmpty()) {
        return {};
    }

    QSqlDatabase db = database();
    if (!db.transaction()) {
        return {};
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO timeline_thumbnail (
            master_video_id,
            timestamp_ms,
            image_path,
            width,
            height,
            size_bytes
        )
        VALUES (
            :master_video_id,
            :timestamp_ms,
            :image_path,
            :width,
            :height,
            :size_bytes
        )
    )");

    for (const TimelineThumbnailCreateInput& item : input) {
        query.bindValue(":master_video_id", item.masterVideoId);
        query.bindValue(":timestamp_ms", item.timestampMs);
        query.bindValue(":image_path", item.imagePath);
        query.bindValue(":width", item.width);
        query.bindValue(":height", item.height);
        query.bindValue(":size_bytes", item.sizeBytes);

        if (!query.exec()) {
            db.rollback();
            return {};
        }
    }

    if (!db.commit()) {
        db.rollback();
        return {};
    }

    return listByMasterVideoId(input.first().masterVideoId);
}

QVector<TimelineThumbnail> TimelineThumbnailRepository::listByMasterVideoId(qint64 masterVideoId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            thumbnail_id,
            master_video_id,
            timestamp_ms,
            image_path,
            width,
            height,
            size_bytes,
            created_at
        FROM timeline_thumbnail
        WHERE master_video_id = :master_video_id
        ORDER BY timestamp_ms ASC
    )");
    query.bindValue(":master_video_id", masterVideoId);

    if (!query.exec()) {
        return {};
    }

    QVector<TimelineThumbnail> thumbnails;
    while (query.next()) {
        thumbnails.append(mapTimelineThumbnail(query));
    }

    return thumbnails;
}

bool TimelineThumbnailRepository::removeByMasterVideoId(qint64 masterVideoId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM timeline_thumbnail WHERE master_video_id = :master_video_id");
    query.bindValue(":master_video_id", masterVideoId);

    return query.exec();
}

} // namespace travis::data::repositories
