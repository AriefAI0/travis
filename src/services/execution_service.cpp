#include "services/execution_service.h"

#include <stdexcept>

namespace travis::services {

namespace {

QString normalizeRequiredText(const QString& value, const char* fieldName) {
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        throw std::runtime_error(fieldName);
    }

    return trimmedValue;
}

std::optional<QString> normalizeOptionalText(const std::optional<QString>& value) {
    if (!value.has_value()) {
        return std::nullopt;
    }

    const QString trimmedValue = value->trimmed();
    if (trimmedValue.isEmpty()) {
        return std::nullopt;
    }

    return trimmedValue;
}

} // namespace

ExecutionService::ExecutionService(
    travis::data::repositories::ExecutionUnitRepository executionUnitRepository,
    travis::data::repositories::ToolingRepository toolingRepository
)
    : executionUnitRepository_(std::move(executionUnitRepository))
    , toolingRepository_(std::move(toolingRepository)) {}

std::optional<travis::models::ExecutionUnit> ExecutionService::createExecutionUnit(
    const travis::models::ExecutionUnitCreateInput& input
) const {
    return executionUnitRepository_.create({
        .type = normalizeRequiredText(input.type, "Execution unit type is required"),
        .name = normalizeRequiredText(input.name, "Execution unit name is required"),
        .meta = normalizeOptionalText(input.meta),
    });
}

QVector<travis::models::ExecutionUnit> ExecutionService::listExecutionUnits() const {
    return executionUnitRepository_.listAll();
}

QVector<travis::models::ExecutionUnit> ExecutionService::listExecutionUnitsByType(const QString& type) const {
    return executionUnitRepository_.listByType(normalizeRequiredText(type, "Execution unit type is required"));
}

std::optional<travis::models::ExecutionUnit> ExecutionService::getExecutionUnitById(qint64 executionUnitId) const {
    return executionUnitRepository_.findById(executionUnitId);
}

std::optional<travis::models::ExecutionUnit> ExecutionService::getExecutionUnitByTypeAndName(
    const QString& type,
    const QString& name
) const {
    return executionUnitRepository_.findByTypeAndName(
        normalizeRequiredText(type, "Execution unit type is required"),
        normalizeRequiredText(name, "Execution unit name is required")
    );
}

std::optional<travis::models::ExecutionUnit> ExecutionService::updateExecutionUnit(
    qint64 executionUnitId,
    const travis::models::ExecutionUnitUpdateInput& input
) const {
    travis::models::ExecutionUnitUpdateInput normalizedInput;

    if (input.type.has_value()) {
        normalizedInput.type = normalizeRequiredText(*input.type, "Execution unit type is required");
    }

    if (input.name.has_value()) {
        normalizedInput.name = normalizeRequiredText(*input.name, "Execution unit name is required");
    }

    if (input.meta.has_value()) {
        normalizedInput.meta = normalizeOptionalText(*input.meta);
    }

    return executionUnitRepository_.updateById(executionUnitId, normalizedInput);
}

bool ExecutionService::deleteExecutionUnit(qint64 executionUnitId) const {
    return executionUnitRepository_.removeById(executionUnitId);
}

std::optional<travis::models::Tooling> ExecutionService::createTooling(
    const travis::models::ToolingCreateInput& input
) const {
    return toolingRepository_.create({
        .executionUnitId = input.executionUnitId,
        .name = normalizeRequiredText(input.name, "Tooling name is required"),
        .config = normalizeOptionalText(input.config),
    });
}

QVector<travis::models::Tooling> ExecutionService::listToolings() const {
    return toolingRepository_.listAll();
}

QVector<travis::models::Tooling> ExecutionService::listToolingsByExecutionUnitId(qint64 executionUnitId) const {
    return toolingRepository_.listByExecutionUnitId(executionUnitId);
}

std::optional<travis::models::Tooling> ExecutionService::getToolingById(qint64 toolingId) const {
    return toolingRepository_.findById(toolingId);
}

std::optional<travis::models::Tooling> ExecutionService::getToolingByExecutionUnitIdAndName(
    qint64 executionUnitId,
    const QString& name
) const {
    return toolingRepository_.findByExecutionUnitIdAndName(
        executionUnitId,
        normalizeRequiredText(name, "Tooling name is required")
    );
}

std::optional<travis::models::Tooling> ExecutionService::updateTooling(
    qint64 toolingId,
    const travis::models::ToolingUpdateInput& input
) const {
    travis::models::ToolingUpdateInput normalizedInput;

    if (input.executionUnitId.has_value()) {
        normalizedInput.executionUnitId = input.executionUnitId;
    }

    if (input.name.has_value()) {
        normalizedInput.name = normalizeRequiredText(*input.name, "Tooling name is required");
    }

    if (input.config.has_value()) {
        normalizedInput.config = normalizeOptionalText(*input.config);
    }

    return toolingRepository_.updateById(toolingId, normalizedInput);
}

bool ExecutionService::deleteTooling(qint64 toolingId) const {
    return toolingRepository_.removeById(toolingId);
}

} // namespace travis::services
