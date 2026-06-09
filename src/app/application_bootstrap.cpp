#include "app/application_bootstrap.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ui/controllers/preview_surface_controller.h"
#include "ui/viewmodels/playback_viewmodel.h"
#include "ui/viewmodels/recording_viewmodel.h"
#include "ui/viewmodels/source_discovery_viewmodel.h"

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
    auto* recordingViewModel =
        new travis::ui::viewmodels::RecordingViewModel(appContext_.recordingWorkflowService(), &engine);
    auto* playbackViewModel =
        new travis::ui::viewmodels::PlaybackViewModel(appContext_.playbackWorkflowService(), &engine);
    auto* sourceDiscoveryViewModel =
        new travis::ui::viewmodels::SourceDiscoveryViewModel(&engine);

    engine.rootContext()->setContextProperty(
        QStringLiteral("previewSurfaceController"),
        previewSurfaceController
    );
    engine.rootContext()->setContextProperty(
        QStringLiteral("recordingViewModel"),
        recordingViewModel
    );
    engine.rootContext()->setContextProperty(
        QStringLiteral("playbackViewModel"),
        playbackViewModel
    );
    engine.rootContext()->setContextProperty(
        QStringLiteral("sourceDiscoveryViewModel"),
        sourceDiscoveryViewModel
    );

    lastError_.clear();
    return true;
}

QString ApplicationBootstrap::lastError() const {
    return lastError_;
}

} // namespace travis::app
