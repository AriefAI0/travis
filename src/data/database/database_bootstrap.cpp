#include "data/database/database_bootstrap.h"

#include "data/database/application_identity.h"

namespace travis::data::database {

bool DatabaseBootstrap::initialize() {
    ApplicationIdentity::apply();

    if (!databaseManager_.open()) {
        lastError_ = databaseManager_.lastError();
        return false;
    }

    lastError_.clear();
    return true;
}

QString DatabaseBootstrap::lastError() const {
    return lastError_;
}

DatabaseManager& DatabaseBootstrap::databaseManager() {
    return databaseManager_;
}

const DatabaseManager& DatabaseBootstrap::databaseManager() const {
    return databaseManager_;
}

} // namespace travis::data::database
