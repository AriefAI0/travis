#include "app/application_bootstrap.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ui/controllers/preview_surface_controller.h"
#include "ui/viewmodels/audio_meter_viewmodel.h"
#include "ui/viewmodels/inspection_context_viewmodel.h"
#include "ui/viewmodels/playback_viewmodel.h"
#include "ui/viewmodels/project_viewmodel.h"
#include "ui/viewmodels/project_workspace_viewmodel.h"
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
    auto* projectViewModel =
        new travis::ui::viewmodels::ProjectViewModel(appContext_.projectService(), &engine);
    auto* projectWorkspaceViewModel =
        new travis::ui::viewmodels::ProjectWorkspaceViewModel(
            appContext_.projectService(),
            appContext_.structureService(),
            appContext_.sessionService(),
            appContext_.videoService(),
            &engine
        );
    auto* inspectionContextViewModel =
        new travis::ui::viewmodels::InspectionContextViewModel(
            appContext_.projectService(),
            appContext_.sessionService(),
            appContext_.structureService(),
            &engine
        );
    auto* playbackViewModel =
        new travis::ui::viewmodels::PlaybackViewModel(appContext_.playbackWorkflowService(), &engine);
    auto* sourceDiscoveryViewModel =
        new travis::ui::viewmodels::SourceDiscoveryViewModel(&engine);
    auto* audioMeterViewModel =
        new travis::ui::viewmodels::AudioMeterViewModel(&engine);

    engine.rootContext()->setContextProperty(
        QStringLiteral("previewSurfaceController"),
        previewSurfaceController
    );
    engine.rootContext()->setContextProperty(
        QStringLiteral("projectViewModel"),
        projectViewModel
    );
    engine.rootContext()->setContextProperty(
        QStringLiteral("projectWorkspaceViewModel"),
        projectWorkspaceViewModel
    );
    engine.rootContext()->setContextProperty(
        QStringLiteral("inspectionContextViewModel"),
        inspectionContextViewModel
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
    engine.rootContext()->setContextProperty(
        QStringLiteral("audioMeterViewModel"),
        audioMeterViewModel
    );

    lastError_.clear();
    return true;
}

QString ApplicationBootstrap::lastError() const {
    return lastError_;
}

} // namespace travis::app
