#include "data/repositories/master_video_repository.h"

#include <QSqlQuery>
#include <QVariant>

namespace travis::data::repositories {

namespace {

using travis::models::MasterVideo;
using travis::models::MasterVideoCreateInput;
using travis::models::MasterVideoUpdateInput;

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

MasterVideo mapMasterVideo(const QSqlQuery& query) {
    return MasterVideo{
        .masterVideoId = query.value("master_video_id").toLongLong(),
        .sessionId = query.value("session_id").toLongLong(),
        .fileUrl = query.value("file_url").toString(),
        .thumbnailUrl = optionalString(query, "thumbnail_url"),
        .startEpoch = query.value("start_epoch").toLongLong(),
        .endEpoch = optionalInteger(query, "end_epoch"),
        .status = query.value("status").toString(),
        .sourceName = optionalString(query, "source_name"),
    };
}

QVariant toNullableString(const std::optional<QString>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::String);
}

QVariant toNullableInteger(const std::optional<qint64>& value) {
    return value ? QVariant(*value) : QVariant(QVariant::LongLong);
}

} // namespace

MasterVideoRepository::MasterVideoRepository(QSqlDatabase database)
    : BaseRepository(std::move(database)) {}

std::optional<MasterVideo> MasterVideoRepository::create(const MasterVideoCreateInput& input) const {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO master_video (
            session_id,
            file_url,
            thumbnail_url,
            start_epoch,
            end_epoch,
            status,
            source_name
        )
        VALUES (
            :session_id,
            :file_url,
            :thumbnail_url,
            :start_epoch,
            :end_epoch,
            :status,
            :source_name
        )
    )");
    query.bindValue(":session_id", input.sessionId);
    query.bindValue(":file_url", input.fileUrl);
    query.bindValue(":thumbnail_url", toNullableString(input.thumbnailUrl));
    query.bindValue(":start_epoch", input.startEpoch);
    query.bindValue(":end_epoch", toNullableInteger(input.endEpoch));
    query.bindValue(":status", input.status.value_or(QStringLiteral("completed")));
    query.bindValue(":source_name", toNullableString(input.sourceName));

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(query.lastInsertId().toLongLong());
}

QVector<MasterVideo> MasterVideoRepository::listAll() const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            master_video_id,
            session_id,
            file_url,
            thumbnail_url,
            start_epoch,
            end_epoch,
            status,
            source_name
        FROM master_video
        ORDER BY session_id ASC, start_epoch ASC, master_video_id ASC
    )");

    if (!query.exec()) {
        return {};
    }

    QVector<MasterVideo> masterVideos;
    while (query.next()) {
        masterVideos.append(mapMasterVideo(query));
    }

    return masterVideos;
}

QVector<MasterVideo> MasterVideoRepository::listBySessionId(qint64 sessionId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            master_video_id,
            session_id,
            file_url,
            thumbnail_url,
            start_epoch,
            end_epoch,
            status,
            source_name
        FROM master_video
        WHERE session_id = :session_id
        ORDER BY start_epoch ASC, master_video_id ASC
    )");
    query.bindValue(":session_id", sessionId);

    if (!query.exec()) {
        return {};
    }

    QVector<MasterVideo> masterVideos;
    while (query.next()) {
        masterVideos.append(mapMasterVideo(query));
    }

    return masterVideos;
}

QVector<MasterVideo> MasterVideoRepository::listByStatus(const QString& status) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            master_video_id,
            session_id,
            file_url,
            thumbnail_url,
            start_epoch,
            end_epoch,
            status,
            source_name
        FROM master_video
        WHERE status = :status
        ORDER BY start_epoch ASC, master_video_id ASC
    )");
    query.bindValue(":status", status);

    if (!query.exec()) {
        return {};
    }

    QVector<MasterVideo> masterVideos;
    while (query.next()) {
        masterVideos.append(mapMasterVideo(query));
    }

    return masterVideos;
}

std::optional<MasterVideo> MasterVideoRepository::findById(qint64 masterVideoId) const {
    QSqlQuery query(database());
    query.prepare(R"(
        SELECT
            master_video_id,
            session_id,
            file_url,
            thumbnail_url,
            start_epoch,
            end_epoch,
            status,
            source_name
        FROM master_video
        WHERE master_video_id = :master_video_id
    )");
    query.bindValue(":master_video_id", masterVideoId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return mapMasterVideo(query);
}

std::optional<MasterVideo> MasterVideoRepository::updateById(
    qint64 masterVideoId,
    const MasterVideoUpdateInput& input
) const {
    const std::optional<MasterVideo> existingMasterVideo = findById(masterVideoId);
    if (!existingMasterVideo.has_value()) {
        return std::nullopt;
    }

    const MasterVideo masterVideo = *existingMasterVideo;

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE master_video
        SET
            session_id = :session_id,
            file_url = :file_url,
            thumbnail_url = :thumbnail_url,
            start_epoch = :start_epoch,
            end_epoch = :end_epoch,
            status = :status,
            source_name = :source_name
        WHERE master_video_id = :master_video_id
    )");
    query.bindValue(":session_id", input.sessionId.value_or(masterVideo.sessionId));
    query.bindValue(":file_url", input.fileUrl.value_or(masterVideo.fileUrl));
    query.bindValue(":thumbnail_url", input.thumbnailUrl ? toNullableString(input.thumbnailUrl) : toNullableString(masterVideo.thumbnailUrl));
    query.bindValue(":start_epoch", input.startEpoch.value_or(masterVideo.startEpoch));
    query.bindValue(":end_epoch", input.endEpoch ? toNullableInteger(input.endEpoch) : toNullableInteger(masterVideo.endEpoch));
    query.bindValue(":status", input.status.value_or(masterVideo.status));
    query.bindValue(":source_name", input.sourceName ? toNullableString(input.sourceName) : toNullableString(masterVideo.sourceName));
    query.bindValue(":master_video_id", masterVideoId);

    if (!query.exec()) {
        return std::nullopt;
    }

    return findById(masterVideoId);
}

bool MasterVideoRepository::removeById(qint64 masterVideoId) const {
    QSqlQuery query(database());
    query.prepare("DELETE FROM master_video WHERE master_video_id = :master_video_id");
    query.bindValue(":master_video_id", masterVideoId);

    return query.exec();
}

} // namespace travis::data::repositories
