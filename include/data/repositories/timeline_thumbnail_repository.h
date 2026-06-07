#pragma once

#include <QVector>

#include "data/repositories/base_repository.h"
#include "models/timeline_thumbnail.h"

// Repository for timeline_thumbnail reads plus batch insert and delete operations.

namespace travis::data::repositories {

class TimelineThumbnailRepository : public BaseRepository {
public:
    explicit TimelineThumbnailRepository(QSqlDatabase database);

    QVector<travis::models::TimelineThumbnail> createMany(
        const travis::models::TimelineThumbnailCreateBatch& input
    ) const;
    QVector<travis::models::TimelineThumbnail> listByMasterVideoId(qint64 masterVideoId) const;
    bool removeByMasterVideoId(qint64 masterVideoId) const;
};

} // namespace travis::data::repositories
