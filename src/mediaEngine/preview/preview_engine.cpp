#include "mediaEngine/preview/preview_engine.h"

#include "mediaEngine/core/native_video_overlay.h"
#include "mediaEngine/core/video_sink_selector.h"

#include <QQuickItem>

// Reuses shared source sessions and attaches a native D3D11 video sink branch for live preview.

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

PreviewResult PreviewEngine::syncPreviewGeometry() {
    if (!activePreview_) {
        return PreviewResult{true, "No active preview to sync"};
    }

    const auto syncResult = travis::media_engine::core::syncNativeVideoOverlayGeometry(
        activePreview_->sink,
        activePreview_->targetItem
    );
    return PreviewResult{syncResult.ok, syncResult.message};
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
    preview.videoConvert = gst_element_factory_make("videoconvert", nullptr);
    preview.sinkKind = sinkSelection.kind;
    preview.sink = sinkSelection.sink;

    if (preview.queue == nullptr || preview.videoConvert == nullptr ||
        preview.sink == nullptr) {
        return PreviewResult{false, "Failed to create preview branch elements"};
    }

    g_object_set(preview.queue, "leaky", 2, "max-size-buffers", 2, nullptr);
    g_object_set(preview.sink, "force-aspect-ratio", TRUE, nullptr);

    const auto geometryResult =
        travis::media_engine::core::syncNativeVideoOverlayGeometry(preview.sink, targetItem);
    if (!geometryResult.ok) {
        return PreviewResult{false, geometryResult.message};
    }

    gst_bin_add_many(
        GST_BIN(session.pipeline),
        preview.queue,
        preview.videoConvert,
        preview.sink,
        nullptr
    );

    const GstStateChangeReturn sinkReadyResult = gst_element_set_state(preview.sink, GST_STATE_READY);
    if (sinkReadyResult == GST_STATE_CHANGE_FAILURE) {
        return PreviewResult{false, "Failed to prepare the selected QML preview sink"};
    }

    bool linkOk = false;
    linkOk = gst_element_link_many(preview.queue, preview.videoConvert, preview.sink, nullptr);

    if (!linkOk) {
        return PreviewResult{false, "Failed to link preview branch elements"};
    }

    const auto attachResult =
        sessionManager_.attachBranch(session, preview.queue, preview.branch);
    if (!attachResult.ok) {
        return PreviewResult{false, attachResult.message};
    }

    bool syncOk = gst_element_sync_state_with_parent(preview.queue) &&
        gst_element_sync_state_with_parent(preview.videoConvert);

    syncOk = syncOk && gst_element_sync_state_with_parent(preview.sink);

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
    if (preview.videoConvert != nullptr) {
        gst_element_set_state(preview.videoConvert, GST_STATE_NULL);
    }
    if (preview.queue != nullptr) {
        gst_element_set_state(preview.queue, GST_STATE_NULL);
    }

    if (preview.session != nullptr) {
        gst_bin_remove_many(
            GST_BIN(preview.session->pipeline),
            preview.queue,
            preview.videoConvert,
            preview.sink,
            nullptr
        );
        sessionManager_.releaseSession(*preview.session);
    }

    activePreview_.reset();

    if (!releaseResult.ok) {
        return PreviewResult{false, releaseResult.message};
    }

    return PreviewResult{true, "Preview stopped"};
}

} // namespace travis::media_engine::preview
