#include "data/database/application_identity.h"

#include <QCoreApplication>

namespace travis::data::database {

void ApplicationIdentity::apply() {
    QCoreApplication::setOrganizationName(QStringLiteral("Trone"));
    QCoreApplication::setApplicationName(QStringLiteral("travis"));
}

} // namespace travis::data::database
