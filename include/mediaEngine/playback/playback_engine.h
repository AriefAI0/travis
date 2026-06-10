#pragma once

#include <QObject>
#include <QPointer>
#include <gst/gst.h>

#include <string>

#include "mediaEngine/core/video_sink_selector.h"

class QQuickItem;

// Owns direct GStreamer playback using the same native sink selection as preview.

namespace travis::media_engine::playback {

struct PlaybackResult {
    bool ok = false;
    std::string message;
};

class PlaybackEngine {
public:
    PlaybackEngine();
    ~PlaybackEngine();

    PlaybackEngine(const PlaybackEngine&) = delete;
    PlaybackEngine& operator=(const PlaybackEngine&) = delete;

    [[nodiscard]] PlaybackResult startFilePlayback(const std::string& filePath, QObject* qmlVideoItem);
    [[nodiscard]] PlaybackResult stopPlayback();
    [[nodiscard]] PlaybackResult syncPlaybackGeometry();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] travis::media_engine::core::PreviewVideoSinkKind activeSinkKind() const;

private:
    PlaybackResult resetPipeline();

    GstElement* pipeline_ = nullptr;
    GstElement* videoSink_ = nullptr;
    QPointer<QQuickItem> playbackItem_;
    travis::media_engine::core::PreviewVideoSinkKind activeSinkKind_ =
        travis::media_engine::core::PreviewVideoSinkKind::D3d11Video;
    bool running_ = false;
};

} // namespace travis::media_engine::playback
