#pragma once

// Declares the shared media-engine runtime boundary for embedded GStreamer initialization.

namespace travis::media_engine::core {

class MediaRuntime {
public:
    MediaRuntime() = default;

    // Returns whether the runtime has been initialized for the current process.
    [[nodiscard]] bool isInitialized() const;

private:
    bool initialized_ = false;
};

} // namespace travis::media_engine::core
