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

    // Creates an execution unit after validating required fields.
    std::optional<travis::models::ExecutionUnit> createExecutionUnit(
        const travis::models::ExecutionUnitCreateInput& input
    ) const;
    // Returns all execution units.
    QVector<travis::models::ExecutionUnit> listExecutionUnits() const;
    // Returns execution units for one type.
    QVector<travis::models::ExecutionUnit> listExecutionUnitsByType(const QString& type) const;
    // Returns one execution unit by id.
    std::optional<travis::models::ExecutionUnit> getExecutionUnitById(qint64 executionUnitId) const;
    // Returns one execution unit by type and name.
    std::optional<travis::models::ExecutionUnit> getExecutionUnitByTypeAndName(
        const QString& type,
        const QString& name
    ) const;
    // Updates one execution unit after validating provided fields.
    std::optional<travis::models::ExecutionUnit> updateExecutionUnit(
        qint64 executionUnitId,
        const travis::models::ExecutionUnitUpdateInput& input
    ) const;
    // Deletes one execution unit by id.
    bool deleteExecutionUnit(qint64 executionUnitId) const;

    // Creates a tooling row after validating required fields.
    std::optional<travis::models::Tooling> createTooling(const travis::models::ToolingCreateInput& input) const;
    // Returns all tooling rows.
    QVector<travis::models::Tooling> listToolings() const;
    // Returns tooling rows for one execution unit.
    QVector<travis::models::Tooling> listToolingsByExecutionUnitId(qint64 executionUnitId) const;
    // Returns one tooling row by id.
    std::optional<travis::models::Tooling> getToolingById(qint64 toolingId) const;
    // Returns one tooling row by execution unit and tooling name.
    std::optional<travis::models::Tooling> getToolingByExecutionUnitIdAndName(
        qint64 executionUnitId,
        const QString& name
    ) const;
    // Updates one tooling row after validating provided fields.
    std::optional<travis::models::Tooling> updateTooling(
        qint64 toolingId,
        const travis::models::ToolingUpdateInput& input
    ) const;
    // Deletes one tooling row by id.
    bool deleteTooling(qint64 toolingId) const;

private:
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository_;
    travis::data::repositories::ToolingRepository toolingRepository_;
};

} // namespace travis::services
