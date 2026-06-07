#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/result.h"

// Repository for CRUD access to the result table.

namespace travis::data::repositories {

class ResultRepository : public BaseRepository {
public:
    explicit ResultRepository(QSqlDatabase database);

    std::optional<travis::models::Result> create(const travis::models::ResultCreateInput& input) const;
    QVector<travis::models::Result> listAll() const;
    QVector<travis::models::Result> listBySessionItemId(qint64 sessionItemId) const;
    std::optional<travis::models::Result> findById(qint64 resultId) const;
    std::optional<travis::models::Result> findBySessionItemInspectionTypeAndStatus(
        qint64 sessionItemId,
        qint64 inspectionTypeId,
        const QString& status
    ) const;
    std::optional<travis::models::Result> updateById(
        qint64 resultId,
        const travis::models::ResultUpdateInput& input
    ) const;
    bool removeById(qint64 resultId) const;
};

} // namespace travis::data::repositories
