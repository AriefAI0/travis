#include "mediaEngine/recording/shared/recording_finalization.h"

#include "mediaEngine/pipeline/errors.h"

#include <chrono>

// Centralizes the EOS wait logic used by recording and clip finalization paths.

namespace travis::media_engine::recording {

namespace {

constexpr auto kRecordingStopTimeout = std::chrono::seconds(30);

} // namespace

RecordingResult waitForFinalization(GstBus* bus, const std::string& successMessage) {
    const auto deadline = std::chrono::steady_clock::now() + kRecordingStopTimeout;

    while (std::chrono::steady_clock::now() < deadline) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            bus,
            100 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
        );

        if (message == nullptr) {
            continue;
        }

        switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            const auto failureMessage = pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
            return RecordingResult{false, failureMessage};
        }
        case GST_MESSAGE_EOS:
            gst_message_unref(message);
            return RecordingResult{true, successMessage};
        default:
            gst_message_unref(message);
            break;
        }
    }

    return RecordingResult{false, "Timed out waiting for recording finalization"};
}

RecordingResult waitForElementMessage(
    GstBus* bus,
    GstElement* target,
    GstMessageType targetMessageType,
    const std::string& successMessage,
    const std::string& timeoutMessage
) {
    const auto deadline = std::chrono::steady_clock::now() + kRecordingStopTimeout;

    while (std::chrono::steady_clock::now() < deadline) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            bus,
            100 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | targetMessageType)
        );

        if (message == nullptr) {
            continue;
        }

        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
            const auto failureMessage = pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
            return RecordingResult{false, failureMessage};
        }

        if (GST_MESSAGE_TYPE(message) == targetMessageType &&
            GST_MESSAGE_SRC(message) == GST_OBJECT(target)) {
            gst_message_unref(message);
            return RecordingResult{true, successMessage};
        }

        gst_message_unref(message);
    }

    return RecordingResult{false, timeoutMessage};
}

GstPadProbeReturn onRecordingSinkEos(
    GstPad*,
    GstPadProbeInfo* info,
    gpointer userData
) {
    auto* context = static_cast<RecordingEosProbeContext*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM) == 0) {
        return GST_PAD_PROBE_OK;
    }

    GstEvent* event = GST_PAD_PROBE_INFO_EVENT(info);
    if (event == nullptr || GST_EVENT_TYPE(event) != GST_EVENT_EOS) {
        return GST_PAD_PROBE_OK;
    }

    {
        std::lock_guard<std::mutex> lock(context->mutex);
        context->done = true;
    }
    context->condition.notify_one();

    return GST_PAD_PROBE_REMOVE;
}

} // namespace travis::media_engine::recording
