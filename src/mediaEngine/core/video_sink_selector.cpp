#include "mediaEngine/core/video_sink_selector.h"

// Creates the fixed qml6glsink preview sink for the Qt/QML rendering path.

namespace travis::media_engine::core {

GstElement* createPreviewVideoSink() {
    return gst_element_factory_make("qml6glsink", nullptr);
}

} // namespace travis::media_engine::core
