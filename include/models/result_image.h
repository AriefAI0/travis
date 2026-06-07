#pragma once

#include <QString>

#include <optional>

// Plain result_image row model mapped from the SQLite result_image table.

namespace travis::models {

struct ResultImage {
    qint64 imageId = 0;
    qint64 resultId = 0;
    QString rawUrl;
    std::optional<QString> annotatedUrl;
    std::optional<QString> remarks;
};

struct ResultImageCreateInput {
    qint64 resultId = 0;
    QString rawUrl;
    std::optional<QString> annotatedUrl;
    std::optional<QString> remarks;
};

struct ResultImageUpdateInput {
    std::optional<qint64> resultId;
    std::optional<QString> rawUrl;
    std::optional<QString> annotatedUrl;
    std::optional<QString> remarks;
};

} // namespace travis::models
