#pragma once

#include <gst/gst.h>

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "mediaEngine/sources/device_capture/device_capture_video_source.h"
#include "mediaEngine/sources/ndi/ndi_video_source.h"

// Shares live source pipelines between preview, playback, and recording consumers.

namespace travis::media_engine::session {

struct MediaSourceIdentity {
    std::string kind;
    std::string sourceName;
    std::string key;
};

struct MediaSourceSessionResult {
    bool ok = false;
    std::string message;
};

struct MediaSourceSession {
    MediaSourceIdentity identity;
    GstElement* pipeline = nullptr;
    travis::media_engine::sources::ndi::NdiVideoSource ndiSource;
    travis::media_engine::sources::device_capture::DeviceCaptureVideoSource deviceCaptureSource;
    GstElement* sourceQueue = nullptr;
    GstElement* convert = nullptr;
    GstElement* tee = nullptr;
    GstElement* warmupQueue = nullptr;
    GstElement* warmupSink = nullptr;
    GstElement* recordingBridgeQueue = nullptr;
    GstElement* recordingBridgeSink = nullptr;
    GstPad* recordingBridgeTeeSrcPad = nullptr;
    std::string recordingBridgeChannel;
    bool linkedVideoPad = false;
    std::atomic<bool> receivedVideoBuffer{false};
    int consumerCount = 0;
    std::optional<std::chrono::steady_clock::time_point> pendingRemovalTime;
};

struct AttachedSessionBranch {
    MediaSourceSession* session = nullptr;
    GstElement* queue = nullptr;
    GstPad* teeSrcPad = nullptr;
};

class MediaSourceSessionManager {
public:
    ~MediaSourceSessionManager();

    [[nodiscard]] MediaSourceSessionResult acquireNdiSession(
        const std::string& sourceName,
        const std::string& urlAddress,
        MediaSourceSession*& session
    );
    [[nodiscard]] MediaSourceSessionResult acquireDeviceCaptureSession(
        const std::string& deviceName,
        const std::string& devicePath,
        const std::string& sourceElement,
        MediaSourceSession*& session
    );
    [[nodiscard]] MediaSourceSessionResult attachBranch(
        MediaSourceSession& session,
        GstElement* queue,
        AttachedSessionBranch& branch
    );
    [[nodiscard]] MediaSourceSessionResult releaseBranch(AttachedSessionBranch& branch);
    [[nodiscard]] MediaSourceSessionResult ensureRecordingBridge(MediaSourceSession& session);
    void releaseSession(MediaSourceSession& session);
    void stopAll();

private:
    std::map<std::string, std::unique_ptr<MediaSourceSession>> sessions_;

    [[nodiscard]] MediaSourceSessionResult createNdiSession(
        const std::string& sourceName,
        const std::string& urlAddress,
        std::unique_ptr<MediaSourceSession>& session
    );
    [[nodiscard]] MediaSourceSessionResult createDeviceCaptureSession(
        const std::string& deviceName,
        const std::string& devicePath,
        const std::string& sourceElement,
        std::unique_ptr<MediaSourceSession>& session
    );
    [[nodiscard]] MediaSourceSessionResult waitForFirstBuffer(MediaSourceSession& session);
    void cleanupExpiredSessions();
    void removeSession(const std::string& sessionKey);
};

[[nodiscard]] MediaSourceIdentity createNdiSourceIdentity(const std::string& sourceName);
[[nodiscard]] MediaSourceIdentity createDeviceCaptureSourceIdentity(
    const std::string& deviceName,
    const std::string& devicePath,
    const std::string& sourceElement
);

} // namespace travis::media_engine::session
