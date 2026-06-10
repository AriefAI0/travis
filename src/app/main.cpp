#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>

#include <gst/gst.h>

#include "app/application_bootstrap.h"

// Starts the first Qt/QML shell used to validate native preview integration.

int main(int argc, char* argv[]) {
    gst_init(nullptr, nullptr);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    travis::app::ApplicationBootstrap bootstrap;
    if (!bootstrap.initialize(engine)) {
        qCritical("Failed to initialize application bootstrap: %s", qPrintable(bootstrap.lastError()));
        return 1;
    }

    engine.loadFromModule("Travis", "Main");
    if (engine.rootObjects().isEmpty()) {
        qCritical("Failed to load main QML module");
        return 1;
    }

    return app.exec();
}
