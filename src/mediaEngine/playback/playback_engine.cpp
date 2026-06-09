#include "mediaEngine/playback/playback_engine.h"

#include <QFileInfo>
#include <QUrl>

// Builds a native playback pipeline for QML-owned video surfaces.

namespace travis::media_engine::playback {

PlaybackEngine::PlaybackEngine() {
    gst_init(nullptr, nullptr);
}

PlaybackEngine::~PlaybackEngine() {
    resetPipeline();
}

PlaybackResult PlaybackEngine::startFilePlayback(const std::string& filePath, QObject* qmlVideoItem) {
    if (filePath.empty()) {
        return PlaybackResult{false, "Playback file path is required"};
    }

    if (qmlVideoItem == nullptr) {
        return PlaybackResult{false, "QML video item is required for native playback"};
    }

    const QFileInfo fileInfo(QString::fromStdString(filePath));
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return PlaybackResult{false, "Playback file does not exist"};
    }

    const PlaybackResult resetResult = resetPipeline();
    if (!resetResult.ok) {
        return resetResult;
    }

    pipeline_ = gst_element_factory_make("playbin", nullptr);
    if (pipeline_ == nullptr) {
        return PlaybackResult{false, "Failed to create GStreamer playbin"};
    }

    const auto sinkSelection = travis::media_engine::core::createPreviewVideoSink();
    if (sinkSelection.sink == nullptr) {
        resetPipeline();
        return PlaybackResult{false, "Failed to create Qt/QML playback video sink"};
    }

    videoSink_ = sinkSelection.sink;
    activeSinkKind_ = sinkSelection.kind;

    // qml6d3d11sink and qml6glsink both render into a QML-owned item through the widget property.
    g_object_set(videoSink_, "widget", qmlVideoItem, nullptr);

    const QUrl fileUrl = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
    g_object_set(
        pipeline_,
        "uri",
        fileUrl.toString(QUrl::FullyEncoded).toUtf8().constData(),
        "video-sink",
        videoSink_,
        nullptr
    );

    const GstStateChangeReturn stateResult = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (stateResult == GST_STATE_CHANGE_FAILURE) {
        resetPipeline();
        return PlaybackResult{false, "Failed to start GStreamer playback pipeline"};
    }

    running_ = true;
    return PlaybackResult{true, "Playback started"};
}

PlaybackResult PlaybackEngine::stopPlayback() {
    return resetPipeline();
}

bool PlaybackEngine::isRunning() const {
    return running_;
}

travis::media_engine::core::PreviewVideoSinkKind PlaybackEngine::activeSinkKind() const {
    return activeSinkKind_;
}

PlaybackResult PlaybackEngine::resetPipeline() {
    running_ = false;

    if (pipeline_ != nullptr) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
        videoSink_ = nullptr;
    }

    return PlaybackResult{true, "Playback stopped"};
}

} // namespace travis::media_engine::playback
