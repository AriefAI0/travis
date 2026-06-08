#include "mediaEngine/core/media_runtime.h"

// Owns the first in-process media-engine runtime hook used by later GStreamer modules.

namespace travis::media_engine::core {

bool MediaRuntime::isInitialized() const {
    return initialized_;
}

} // namespace travis::media_engine::core
