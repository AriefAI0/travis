#include "mediaEngine/preview/preview_engine.h"

#include "mediaEngine/core/video_sink_selector.h"

#include <QQuickItem>

// Reuses shared source sessions and attaches the selected Qt/QML sink branch for live preview.

namespace travis::media_engine::preview {

PreviewEngine::PreviewEngine(travis::media_engine::session::MediaSourceSessionManager& sessionManager)
    : sessionManager_(sessionManager) {}

PreviewEngine::~PreviewEngine() {
    (void)clearActivePreview();
}

PreviewResult PreviewEngine::startPreview(
    const std::string& sourceKind,
    const std::string& sourceName,
    const std::string& urlAddress,
    const std::string& devicePath,
    const std::string& sourceElement,
    QQuickItem* targetItem
) {
    if (sourceName.empty()) {
        return PreviewResult{false, "sourceName is required"};
    }

    if (targetItem == nullptr) {
        return PreviewResult{false, "A QML preview item is required"};
    }

    if (activePreview_ &&
        activePreview_->sourceKind == sourceKind &&
        activePreview_->sourceName == sourceName &&
        activePreview_->targetItem == targetItem) {
        return PreviewResult{true, "Preview already active"};
    }

    const auto stopResult = clearActivePreview();
    if (!stopResult.ok) {
        return stopResult;
    }

    travis::media_engine::session::MediaSourceSession* session = nullptr;
    const auto sessionResult = acquireSourceSession(
        sourceKind,
        sourceName,
        urlAddress,
        devicePath,
        sourceElement,
        session
    );

    if (!sessionResult.ok || session == nullptr) {
        return PreviewResult{false, sessionResult.message};
    }

    activePreview_ = std::make_unique<ActivePreview>(ActivePreview{
        .sourceKind = sourceKind,
        .sourceName = sourceName,
        .targetItem = targetItem,
        .session = session,
    });

    const auto createResult = createPreviewBranch(*session, targetItem);
    if (!createResult.ok) {
        (void)clearActivePreview();
        return createResult;
    }

    return PreviewResult{true, "Preview started"};
}

PreviewResult PreviewEngine::stopPreview() {
    return clearActivePreview();
}

bool PreviewEngine::hasActivePreview() const {
    return activePreview_ != nullptr;
}

travis::media_engine::session::MediaSourceSessionResult PreviewEngine::acquireSourceSession(
    const std::string& sourceKind,
    const std::string& sourceName,
    const std::string& urlAddress,
    const std::string& devicePath,
    const std::string& sourceElement,
    travis::media_engine::session::MediaSourceSession*& session
) {
    if (sourceKind == "device-capture") {
        return sessionManager_.acquireDeviceCaptureSession(
            sourceName,
            devicePath,
            sourceElement,
            session
        );
    }

    if (sourceKind == "ndi") {
        return sessionManager_.acquireNdiSession(sourceName, urlAddress, session);
    }

    session = nullptr;
    return travis::media_engine::session::MediaSourceSessionResult{
        false,
        "Preview source is not supported yet: " + sourceKind,
    };
}

PreviewResult PreviewEngine::createPreviewBranch(
    travis::media_engine::session::MediaSourceSession& session,
    QQuickItem* targetItem
) {
    ActivePreview& preview = *activePreview_;
    const auto sinkSelection = travis::media_engine::core::createPreviewVideoSink();

    preview.queue = gst_element_factory_make("queue", nullptr);
    preview.sinkKind = sinkSelection.kind;
    preview.sink = sinkSelection.sink;

    if (preview.sinkKind == travis::media_engine::core::PreviewVideoSinkKind::Qml6Gl) {
        preview.glUpload = gst_element_factory_make("glupload", nullptr);
        preview.glColorConvert = gst_element_factory_make("glcolorconvert", nullptr);
    }

    if (preview.queue == nullptr ||
        preview.sink == nullptr) {
        return PreviewResult{false, "Failed to create preview branch elements"};
    }

    if (preview.sinkKind == travis::media_engine::core::PreviewVideoSinkKind::Qml6Gl &&
        (preview.glUpload == nullptr || preview.glColorConvert == nullptr)) {
        return PreviewResult{false, "Failed to create the OpenGL preview path"};
    }

    g_object_set(preview.queue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(preview.sink, "widget", targetItem, "force-aspect-ratio", TRUE, nullptr);

    if (preview.sinkKind == travis::media_engine::core::PreviewVideoSinkKind::Qml6Gl) {
        gst_bin_add_many(
            GST_BIN(session.pipeline),
            preview.queue,
            preview.glUpload,
            preview.glColorConvert,
            preview.sink,
            nullptr
        );
    } else {
        gst_bin_add_many(
            GST_BIN(session.pipeline),
            preview.queue,
            preview.sink,
            nullptr
        );
    }

    const GstStateChangeReturn sinkReadyResult = gst_element_set_state(preview.sink, GST_STATE_READY);
    if (sinkReadyResult == GST_STATE_CHANGE_FAILURE) {
        return PreviewResult{false, "Failed to prepare the selected QML preview sink"};
    }

    bool linkOk = false;
    if (preview.sinkKind == travis::media_engine::core::PreviewVideoSinkKind::Qml6Gl) {
        linkOk = gst_element_link_many(
            preview.queue,
            preview.glUpload,
            preview.glColorConvert,
            preview.sink,
            nullptr
        );
    } else {
        linkOk = gst_element_link(preview.queue, preview.sink);
    }

    if (!linkOk) {
        return PreviewResult{false, "Failed to link preview branch elements"};
    }

    const auto attachResult =
        sessionManager_.attachBranch(session, preview.queue, preview.branch);
    if (!attachResult.ok) {
        return PreviewResult{false, attachResult.message};
    }

    bool syncOk = gst_element_sync_state_with_parent(preview.sink) &&
        gst_element_sync_state_with_parent(preview.queue);

    if (preview.sinkKind == travis::media_engine::core::PreviewVideoSinkKind::Qml6Gl) {
        syncOk = syncOk &&
            gst_element_sync_state_with_parent(preview.glColorConvert) &&
            gst_element_sync_state_with_parent(preview.glUpload);
    }

    if (!syncOk) {
        return PreviewResult{false, "Failed to sync preview branch with live source session"};
    }

    return PreviewResult{true, "Preview branch attached"};
}

PreviewResult PreviewEngine::clearActivePreview() {
    if (!activePreview_) {
        return PreviewResult{true, "No active preview to stop"};
    }

    ActivePreview& preview = *activePreview_;

    const auto releaseResult = sessionManager_.releaseBranch(preview.branch);

    if (preview.sink != nullptr) {
        gst_element_set_state(preview.sink, GST_STATE_NULL);
    }
    if (preview.glColorConvert != nullptr) {
        gst_element_set_state(preview.glColorConvert, GST_STATE_NULL);
    }
    if (preview.glUpload != nullptr) {
        gst_element_set_state(preview.glUpload, GST_STATE_NULL);
    }
    if (preview.queue != nullptr) {
        gst_element_set_state(preview.queue, GST_STATE_NULL);
    }

    if (preview.session != nullptr) {
        if (preview.sinkKind == travis::media_engine::core::PreviewVideoSinkKind::Qml6Gl) {
            gst_bin_remove_many(
                GST_BIN(preview.session->pipeline),
                preview.queue,
                preview.glUpload,
                preview.glColorConvert,
                preview.sink,
                nullptr
            );
        } else {
            gst_bin_remove_many(
                GST_BIN(preview.session->pipeline),
                preview.queue,
                preview.sink,
                nullptr
            );
        }
        sessionManager_.releaseSession(*preview.session);
    }

    activePreview_.reset();

    if (!releaseResult.ok) {
        return PreviewResult{false, releaseResult.message};
    }

    return PreviewResult{true, "Preview stopped"};
}

} // namespace travis::media_engine::preview
