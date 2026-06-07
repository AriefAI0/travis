#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/tooling.h"

// Repository for CRUD access to the tooling table.

namespace travis::data::repositories {

class ToolingRepository : public BaseRepository {
public:
    explicit ToolingRepository(QSqlDatabase database);

    std::optional<travis::models::Tooling> create(const travis::models::ToolingCreateInput& input) const;
    QVector<travis::models::Tooling> listAll() const;
    QVector<travis::models::Tooling> listByExecutionUnitId(qint64 executionUnitId) const;
    std::optional<travis::models::Tooling> findById(qint64 toolingId) const;
    std::optional<travis::models::Tooling> findByExecutionUnitIdAndName(qint64 executionUnitId, const QString& name) const;
    std::optional<travis::models::Tooling> updateById(
        qint64 toolingId,
        const travis::models::ToolingUpdateInput& input
    ) const;
    bool removeById(qint64 toolingId) const;
};

} // namespace travis::data::repositories
