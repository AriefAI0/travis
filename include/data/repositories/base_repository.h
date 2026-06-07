#pragma once

#include <QSqlDatabase>

// Shared repository base that gives child repositories access to the opened Qt SQL connection.

namespace travis::data::repositories {

class BaseRepository {
public:
    explicit BaseRepository(QSqlDatabase database);
    virtual ~BaseRepository() = default;

protected:
    QSqlDatabase database() const;

private:
    QSqlDatabase database_;
};

} // namespace travis::data::repositories
