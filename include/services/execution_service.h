#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/execution_unit_repository.h"
#include "data/repositories/tooling_repository.h"
#include "models/execution_unit.h"
#include "models/tooling.h"

// Service layer for execution unit and tooling validation plus orchestration.

namespace travis::services {

class ExecutionService {
public:
    ExecutionService(
        travis::data::repositories::ExecutionUnitRepository executionUnitRepository,
        travis::data::repositories::ToolingRepository toolingRepository
    );

    std::optional<travis::models::ExecutionUnit> createExecutionUnit(
        const travis::models::ExecutionUnitCreateInput& input
    ) const;
    QVector<travis::models::ExecutionUnit> listExecutionUnits() const;
    QVector<travis::models::ExecutionUnit> listExecutionUnitsByType(const QString& type) const;
    std::optional<travis::models::ExecutionUnit> getExecutionUnitById(qint64 executionUnitId) const;
    std::optional<travis::models::ExecutionUnit> getExecutionUnitByTypeAndName(
        const QString& type,
        const QString& name
    ) const;
    std::optional<travis::models::ExecutionUnit> updateExecutionUnit(
        qint64 executionUnitId,
        const travis::models::ExecutionUnitUpdateInput& input
    ) const;
    bool deleteExecutionUnit(qint64 executionUnitId) const;

    std::optional<travis::models::Tooling> createTooling(const travis::models::ToolingCreateInput& input) const;
    QVector<travis::models::Tooling> listToolings() const;
    QVector<travis::models::Tooling> listToolingsByExecutionUnitId(qint64 executionUnitId) const;
    std::optional<travis::models::Tooling> getToolingById(qint64 toolingId) const;
    std::optional<travis::models::Tooling> getToolingByExecutionUnitIdAndName(
        qint64 executionUnitId,
        const QString& name
    ) const;
    std::optional<travis::models::Tooling> updateTooling(
        qint64 toolingId,
        const travis::models::ToolingUpdateInput& input
    ) const;
    bool deleteTooling(qint64 toolingId) const;

private:
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository_;
    travis::data::repositories::ToolingRepository toolingRepository_;
};

} // namespace travis::services
