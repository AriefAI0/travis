#include "app/application_bootstrap.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ui/controllers/preview_surface_controller.h"

// Publishes the first native preview controller to QML and keeps startup validation centralized.

namespace travis::app {

ApplicationBootstrap::ApplicationBootstrap() = default;

bool ApplicationBootstrap::initialize(QQmlApplicationEngine& engine) {
    if (!appContext_.initialize()) {
        lastError_ = appContext_.lastError();
        return false;
    }

    auto* previewSurfaceController =
        new travis::ui::controllers::PreviewSurfaceController(&engine);

    engine.rootContext()->setContextProperty(
        QStringLiteral("previewSurfaceController"),
        previewSurfaceController
    );

    lastError_.clear();
    return true;
}

QString ApplicationBootstrap::lastError() const {
    return lastError_;
}

} // namespace travis::app
