#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/result_image.h"

// Repository for CRUD access to the result_image table.

namespace travis::data::repositories {

class ResultImageRepository : public BaseRepository {
public:
    explicit ResultImageRepository(QSqlDatabase database);

    std::optional<travis::models::ResultImage> create(const travis::models::ResultImageCreateInput& input) const;
    QVector<travis::models::ResultImage> listAll() const;
    QVector<travis::models::ResultImage> listByResultId(qint64 resultId) const;
    std::optional<travis::models::ResultImage> findById(qint64 imageId) const;
    std::optional<travis::models::ResultImage> updateById(
        qint64 imageId,
        const travis::models::ResultImageUpdateInput& input
    ) const;
    bool removeById(qint64 imageId) const;
};

} // namespace travis::data::repositories
