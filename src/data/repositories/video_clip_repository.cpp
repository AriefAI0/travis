#include "data/repositories/video_clip_repository.h"

#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::VideoClip;
using travis::models::VideoClipCreateInput;
using travis::models::VideoClipUpdateInput;

std::optional<QString> optionalString(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toString();
}

std::optional<qint64> optionalInteger(const QSqlQuery& query, const char* columnName) {
    const QVariant value = query.value(columnName);
    if (!value.isValid() || value.isNull()) {
        return std::nullopt;
    }

    return value.toLongLong();
}

VideoClip mapVideoClip(const QSqlQuery& query) {
    return VideoClip{
        .clipId = query.value("clip_id").toLongLong(),
        .resultId = query.value("result_id").toLongLong(),
        .masterVideoId = query.value("master_video_id").toLongLong(),
        .startOffsetMs = query.value("start_offset_ms").toLongLong(),
        .endOffsetMs = optionalInteger(query, "end_offset_ms"),
        .clipFileUrl = optionalString(query, "clip_file_url"),
        .thumbnailUrl = optionalString(query, "thumbnail_url"),
        .status = query.value("status").toString(),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

QVariant toNullableInteger(const std::optional<qint64>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::LongLong);
}

} // namespace

VideoClipRepository::VideoClipRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<VideoClip> VideoClipRepository::create(const VideoClipCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO video_clip (
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        )
        VALUES (
            :result_id,
            :master_video_id,
            :start_offset_ms,
            :end_offset_ms,
            :clip_file_url,
            :thumbnail_url,
            :status
        )
    )");
    query.bindValue(":result_id", input.resultId);
    query.bindValue(":master_video_id", input.masterVideoId);
    query.bindValue(":start_offset_ms", input.startOffsetMs);
    query.bindValue(":end_offset_ms", toNullableInteger(input.endOffsetMs));
    query.bindValue(":clip_file_url", toNullableString(input.clipFileUrl));
    query.bindValue(":thumbnail_url", toNullableString(input.thumbnailUrl));
    query.bindValue(":status", input.status.value_or(QStringLiteral("completed")));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<VideoClip> VideoClipRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            clip_id,
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        FROM video_clip
        ORDER BY clip_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<VideoClip> videoClips;
    while (query.next()) {
        videoClips.append(mapVideoClip(query));
    }

    return videoClips;
}

QVector<VideoClip> VideoClipRepository::listByResultId(qint64 resultId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            clip_id,
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        FROM video_clip
        WHERE result_id = :result_id
        ORDER BY start_offset_ms ASC
    )");
    query.bindValue(":result_id", resultId);

    if (!query.exec()) {
        return {};
    }

    QVector<VideoClip> videoClips;
    while (query.next()) {
        videoClips.append(mapVideoClip(query));
    }

    return videoClips;
}

QVector<VideoClip> VideoClipRepository::listByMasterVideoId(qint64 masterVideoId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            clip_id,
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        FROM video_clip
        WHERE master_video_id = :master_video_id
        ORDER BY start_offset_ms ASC
    )");
    query.bindValue(":master_video_id", masterVideoId);

    if (!query.exec()) {
        return {};
    }

    QVector<VideoClip> videoClips;
    while (query.next()) {
        videoClips.append(mapVideoClip(query));
    }

    return videoClips;
}

QVector<VideoClip> VideoClipRepository::listByStatus(const QString& status) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            clip_id,
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        FROM video_clip
        WHERE status = :status
        ORDER BY clip_id ASC
    )");
    query.bindValue(":status", status);

    if (!query.exec()) {
        return {};
    }

    QVector<VideoClip> videoClips;
    while (query.next()) {
        videoClips.append(mapVideoClip(query));
    }

    return videoClips;
}

std::optional<VideoClip> VideoClipRepository::findById(qint64 clipId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            clip_id,
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        FROM video_clip
        WHERE clip_id = :clip_id
    )");
    query.bindValue(":clip_id", clipId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapVideoClip(query);
}

std::optional<VideoClip> VideoClipRepository::findActiveByResultId(qint64 resultId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            clip_id,
            result_id,
            master_video_id,
            start_offset_ms,
            end_offset_ms,
            clip_file_url,
            thumbnail_url,
            status
        FROM video_clip
        WHERE result_id = :result_id AND end_offset_ms IS NULL
        ORDER BY start_offset_ms ASC
        LIMIT 1
    )");
    query.bindValue(":result_id", resultId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapVideoClip(query);
}

std::optional<VideoClip> VideoClipRepository::updateById(qint64 clipId, const VideoClipUpdateInput& input) const {
    const std::optional<VideoClip> existingVideoClip = findById(clipId);
    if (!existingVideoClip.has_value()) {
        return std::nullopt;
    }

    const VideoClip videoClip = *existingVideoClip;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE video_clip
        SET
            result_id = :result_id,
            master_video_id = :master_video_id,
            start_offset_ms = :start_offset_ms,
            end_offset_ms = :end_offset_ms,
            clip_file_url = :clip_file_url,
            thumbnail_url = :thumbnail_url,
            status = :status
        WHERE clip_id = :clip_id
    )");
    query.bindValue(":result_id", input.resultId.value_or(videoClip.resultId));
    query.bindValue(":master_video_id", input.masterVideoId.value_or(videoClip.masterVideoId));
    query.bindValue(":start_offset_ms", input.startOffsetMs.value_or(videoClip.startOffsetMs));
    query.bindValue(":end_offset_ms", input.endOffsetMs ? toNullableInteger(input.endOffsetMs) : toNullableInteger(videoClip.endOffsetMs));
    query.bindValue(":clip_file_url", input.clipFileUrl ? toNullableString(input.clipFileUrl) : toNullableString(videoClip.clipFileUrl));
    query.bindValue(":thumbnail_url", input.thumbnailUrl ? toNullableString(input.thumbnailUrl) : toNullableString(videoClip.thumbnailUrl));
    query.bindValue(":status", input.status.value_or(videoClip.status));
    query.bindValue(":clip_id", clipId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(clipId);
}

bool VideoClipRepository::removeById(qint64 clipId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM video_clip WHERE clip_id = :clip_id");
    query.bindValue(":clip_id", clipId);

    return query.exec();
}

} // namespace travis::data::repositories
