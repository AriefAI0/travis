#pragma once

#include <QString>

#include "app/app_context.h"

namespace travis::ui::controllers {
class PreviewSurfaceController;
}

class QQmlApplicationEngine;

// Builds the first Qt/QML application shell and exposes native controllers to QML.

namespace travis::app {

class ApplicationBootstrap {
public:
    ApplicationBootstrap();

    // Initializes shared dependencies and publishes the first UI bridge objects to QML.
    bool initialize(QQmlApplicationEngine& engine);

    [[nodiscard]] QString lastError() const;

private:
    AppContext appContext_;
    QString lastError_;
};

} // namespace travis::app
