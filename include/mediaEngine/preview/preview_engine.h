#pragma once

#include <gst/gst.h>

#include <memory>
#include <string>

#include <QPointer>

#include "mediaEngine/core/video_sink_selector.h"
#include "mediaEngine/session/media_source_session.h"

class QQuickItem;

// Owns the native preview branch that connects shared media sessions to the selected native video sink.

namespace travis::media_engine::preview {

struct PreviewResult {
    bool ok = false;
    std::string message;
};

class PreviewEngine {
public:
    explicit PreviewEngine(travis::media_engine::session::MediaSourceSessionManager& sessionManager);
    ~PreviewEngine();

    [[nodiscard]] PreviewResult startPreview(
        const std::string& sourceKind,
        const std::string& sourceName,
        const std::string& urlAddress,
        const std::string& devicePath,
        const std::string& sourceElement,
        QQuickItem* targetItem
    );
    [[nodiscard]] PreviewResult stopPreview();
    [[nodiscard]] PreviewResult syncPreviewGeometry();
    [[nodiscard]] bool hasActivePreview() const;

private:
    struct ActivePreview {
        std::string sourceKind;
        std::string sourceName;
        QPointer<QQuickItem> targetItem;
        travis::media_engine::session::MediaSourceSession* session = nullptr;
        GstElement* queue = nullptr;
        travis::media_engine::core::PreviewVideoSinkKind sinkKind =
            travis::media_engine::core::PreviewVideoSinkKind::D3d11Video;
        GstElement* videoConvert = nullptr;
        GstElement* sink = nullptr;
        travis::media_engine::session::AttachedSessionBranch branch;
    };

    [[nodiscard]] travis::media_engine::session::MediaSourceSessionResult acquireSourceSession(
        const std::string& sourceKind,
        const std::string& sourceName,
        const std::string& urlAddress,
        const std::string& devicePath,
        const std::string& sourceElement,
        travis::media_engine::session::MediaSourceSession*& session
    );
    [[nodiscard]] PreviewResult createPreviewBranch(
        travis::media_engine::session::MediaSourceSession& session,
        QQuickItem* targetItem
    );
    [[nodiscard]] PreviewResult clearActivePreview();

    travis::media_engine::session::MediaSourceSessionManager& sessionManager_;
    std::unique_ptr<ActivePreview> activePreview_;
};

} // namespace travis::media_engine::preview
