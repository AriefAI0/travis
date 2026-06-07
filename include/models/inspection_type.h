#pragma once

#include <QString>

#include <optional>

// Plain inspection_type row model mapped from the SQLite inspection_type table.

namespace travis::models {

struct InspectionType {
    qint64 inspectionTypeId = 0;
    QString name;
};

struct InspectionTypeCreateInput {
    QString name;
};

struct InspectionTypeUpdateInput {
    std::optional<QString> name;
};

} // namespace travis::models
