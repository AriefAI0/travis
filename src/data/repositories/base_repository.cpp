#include "data/repositories/base_repository.h"

namespace travis::data::repositories {

BaseRepository::BaseRepository(QSqlDatabase database)
    : database_(std::move(database)) {}

QSqlDatabase BaseRepository::database() const {
    return database_;
}

} // namespace travis::data::repositories
