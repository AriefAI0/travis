#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/execution_unit.h"

// Repository for CRUD access to the execution_unit table.

namespace travis::data::repositories {

class ExecutionUnitRepository : public BaseRepository {
public:
    explicit ExecutionUnitRepository(QSqlDatabase database);

    std::optional<travis::models::ExecutionUnit> create(const travis::models::ExecutionUnitCreateInput& input) const;
    QVector<travis::models::ExecutionUnit> listAll() const;
    QVector<travis::models::ExecutionUnit> listByType(const QString& type) const;
    std::optional<travis::models::ExecutionUnit> findById(qint64 executionUnitId) const;
    std::optional<travis::models::ExecutionUnit> findByTypeAndName(const QString& type, const QString& name) const;
    std::optional<travis::models::ExecutionUnit> updateById(
        qint64 executionUnitId,
        const travis::models::ExecutionUnitUpdateInput& input
    ) const;
    bool removeById(qint64 executionUnitId) const;
};

} // namespace travis::data::repositories
