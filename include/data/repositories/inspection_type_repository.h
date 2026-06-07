#pragma once

#include <QVector>

#include <optional>

#include "data/repositories/base_repository.h"
#include "models/inspection_type.h"

// Repository for CRUD access to the inspection_type table.

namespace travis::data::repositories {

class InspectionTypeRepository : public BaseRepository {
public:
    explicit InspectionTypeRepository(QSqlDatabase database);

    std::optional<travis::models::InspectionType> create(const travis::models::InspectionTypeCreateInput& input) const;
    std::optional<travis::models::InspectionType> createIfMissing(const travis::models::InspectionTypeCreateInput& input) const;
    QVector<travis::models::InspectionType> listAll() const;
    std::optional<travis::models::InspectionType> findById(qint64 inspectionTypeId) const;
    std::optional<travis::models::InspectionType> findByName(const QString& name) const;
    std::optional<travis::models::InspectionType> updateById(
        qint64 inspectionTypeId,
        const travis::models::InspectionTypeUpdateInput& input
    ) const;
    bool removeById(qint64 inspectionTypeId) const;
};

} // namespace travis::data::repositories
