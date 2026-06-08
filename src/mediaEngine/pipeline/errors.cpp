#include "mediaEngine/pipeline/errors.h"

// Converts GStreamer error and debug information into a single loggable string.

namespace travis::media_engine::pipeline {

std::string readGstErrorMessage(GstMessage* message) {
    GError* error = nullptr;
    gchar* debugInfo = nullptr;
    gst_message_parse_error(message, &error, &debugInfo);

    std::string messageText = error != nullptr ? error->message : "Unknown GStreamer error";

    if (debugInfo != nullptr && std::string(debugInfo).length() > 0) {
        messageText += ": ";
        messageText += debugInfo;
    }

    if (error != nullptr) {
        g_error_free(error);
    }

    if (debugInfo != nullptr) {
        g_free(debugInfo);
    }

    return messageText;
}

} // namespace travis::media_engine::pipeline
