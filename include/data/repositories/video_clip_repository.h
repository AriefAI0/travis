#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/video_clip.h"

// Repository for CRUD access to the video_clip table.

namespace travis::data::repositories {

class VideoClipRepository : public BaseRepository {
public:
    explicit VideoClipRepository(QSqlDatabase database);

    std::optional<travis::models::VideoClip> create(const travis::models::VideoClipCreateInput& input) const;
    QVector<travis::models::VideoClip> listAll() const;
    QVector<travis::models::VideoClip> listByResultId(qint64 resultId) const;
    QVector<travis::models::VideoClip> listByMasterVideoId(qint64 masterVideoId) const;
    QVector<travis::models::VideoClip> listByStatus(const QString& status) const;
    std::optional<travis::models::VideoClip> findById(qint64 clipId) const;
    std::optional<travis::models::VideoClip> findActiveByResultId(qint64 resultId) const;
    std::optional<travis::models::VideoClip> updateById(
        qint64 clipId,
        const travis::models::VideoClipUpdateInput& input
    ) const;
    bool removeById(qint64 clipId) const;
};

} // namespace travis::data::repositories
