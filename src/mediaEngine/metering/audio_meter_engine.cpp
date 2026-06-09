#include "mediaEngine/metering/audio_meter_engine.h"

#include "mediaEngine/pipeline/errors.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

// Builds independent audio meter pipelines and reports processed peak levels through a callback.

namespace travis::media_engine::metering {

namespace {

constexpr double kMinimumDb = -60.0;
constexpr GstClockTime kAudioMeterStartTimeout = 5 * GST_SECOND;
constexpr GstClockTime kAudioMeterProbeInterval = 50 * GST_MSECOND;

double clampDb(double value) {
    if (!std::isfinite(value)) {
        return kMinimumDb;
    }

    return std::max(kMinimumDb, std::min(0.0, value));
}

int dbToLevel(double value) {
    const double clamped = clampDb(value);
    const double normalized = ((clamped - kMinimumDb) / std::abs(kMinimumDb)) * 100.0;
    return static_cast<int>(normalized + 0.5);
}

double clampPercent(double value) {
    if (!std::isfinite(value)) {
        return 0.0;
    }

    return std::max(0.0, std::min(100.0, value));
}

double clampBalance(double value) {
    if (!std::isfinite(value)) {
        return 0.0;
    }

    return std::max(-100.0, std::min(100.0, value));
}

double balanceGainForLeft(double balance) {
    return balance > 0.0 ? 1.0 - (balance / 100.0) : 1.0;
}

double balanceGainForRight(double balance) {
    return balance < 0.0 ? 1.0 + (balance / 100.0) : 1.0;
}

std::string resolveAudioSourceElement(const std::string& sourceElement) {
    return sourceElement.empty() ? "wasapi2src" : sourceElement;
}

void setAudioSourceDevice(
    GstElement* source,
    const std::string& sourceElement,
    const std::string& deviceName,
    const std::string& devicePath
) {
    if (sourceElement == "wasapi2src" || sourceElement == "wasapisrc") {
        if (!devicePath.empty()) {
            g_object_set(source, "device", devicePath.c_str(), nullptr);
        }

        if (sourceElement == "wasapi2src") {
            g_object_set(
                source,
                "low-latency",
                TRUE,
                "latency-time",
                static_cast<gint64>(5000),
                "buffer-time",
                static_cast<gint64>(20000),
                "do-timestamp",
                TRUE,
                nullptr
            );
        }
        return;
    }

    if (!devicePath.empty()) {
        g_object_set(source, "device", devicePath.c_str(), nullptr);
        return;
    }

    if (!deviceName.empty()) {
        g_object_set(source, "device-name", deviceName.c_str(), nullptr);
    }
}

GstPadProbeReturn onAudioMeterBuffer(GstPad* pad, GstPadProbeInfo* info, gpointer userData) {
    auto* meter = static_cast<RunningAudioMeter*>(userData);

    if ((GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) == 0) {
        return GST_PAD_PROBE_OK;
    }

    GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (buffer == nullptr) {
        return GST_PAD_PROBE_OK;
    }

    const GstClockTime now = gst_util_get_timestamp();
    if (meter->lastProbeEmitTime != GST_CLOCK_TIME_NONE &&
        now - meter->lastProbeEmitTime < kAudioMeterProbeInterval) {
        return GST_PAD_PROBE_OK;
    }

    GstCaps* caps = gst_pad_get_current_caps(pad);
    if (caps == nullptr) {
        return GST_PAD_PROBE_OK;
    }

    const GstStructure* structure = gst_caps_get_structure(caps, 0);
    const gchar* format = gst_structure_get_string(structure, "format");
    gint channels = 0;
    gst_structure_get_int(structure, "channels", &channels);
    gst_caps_unref(caps);

    if (format == nullptr || std::string(format) != "S16LE" || channels <= 0) {
        return GST_PAD_PROBE_OK;
    }

    GstMapInfo mapInfo;
    if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
        return GST_PAD_PROBE_OK;
    }

    const auto* samples = reinterpret_cast<const std::int16_t*>(mapInfo.data);
    const std::size_t sampleCount = mapInfo.size / sizeof(std::int16_t);
    double leftPeak = 0.0;
    double rightPeak = 0.0;
    double volume = 1.0;
    double balance = 0.0;
    bool mono = false;

    {
        std::lock_guard<std::mutex> lock(meter->settingsMutex);
        volume = clampPercent(meter->volume) / 100.0;
        balance = clampBalance(meter->balance);
        mono = meter->mono;
    }

    const double leftGain = volume * balanceGainForLeft(balance);
    const double rightGain = volume * balanceGainForRight(balance);

    for (std::size_t index = 0; index < sampleCount; index += static_cast<std::size_t>(channels)) {
        double leftSample = samples[index] / 32768.0;
        double rightSample = leftSample;

        if (channels > 1 && index + 1 < sampleCount) {
            rightSample = samples[index + 1] / 32768.0;
        }

        if (mono) {
            const double monoSample = (leftSample + rightSample) / 2.0;
            leftSample = monoSample;
            rightSample = monoSample;
        }

        leftPeak = std::max(leftPeak, std::abs(leftSample * leftGain));
        rightPeak = std::max(rightPeak, std::abs(rightSample * rightGain));
    }

    gst_buffer_unmap(buffer, &mapInfo);

    const double leftDb = leftPeak > 0.0 ? 20.0 * std::log10(leftPeak) : kMinimumDb;
    const double rightDb = rightPeak > 0.0 ? 20.0 * std::log10(rightPeak) : kMinimumDb;
    meter->lastProbeEmitTime = now;

    if (meter->owner != nullptr) {
        meter->owner->emitLevel(AudioMeterLevel{
            .meterId = meter->meterId,
            .leftLevel = dbToLevel(leftDb),
            .rightLevel = dbToLevel(rightDb),
            .leftPeakDb = clampDb(leftDb),
            .rightPeakDb = clampDb(rightDb),
        });
    }

    return GST_PAD_PROBE_OK;
}

void handleAudioMeterMessage(GstMessage* message) {
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        std::cerr << travis::media_engine::pipeline::readGstErrorMessage(message) << std::endl;
    }
}

void pollAudioMeterBus(RunningAudioMeter* meter) {
    while (!meter->stopRequested.load()) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            meter->bus,
            100 * GST_MSECOND,
            GST_MESSAGE_ERROR
        );

        if (message == nullptr) {
            continue;
        }

        handleAudioMeterMessage(message);
        gst_message_unref(message);
    }
}

} // namespace

AudioMeterEngine::AudioMeterEngine() {
    gst_init(nullptr, nullptr);
}

AudioMeterEngine::~AudioMeterEngine() {
    stopAll();
}

void AudioMeterEngine::setLevelCallback(LevelCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    levelCallback_ = std::move(callback);
}

AudioMeterResult AudioMeterEngine::startMeter(const AudioMeterInput& input) {
    if (input.meterId.empty()) {
        return AudioMeterResult{false, "meterId is required"};
    }

    if (input.deviceName.empty()) {
        return AudioMeterResult{false, "deviceName is required"};
    }

    std::lock_guard<std::mutex> lock(metersMutex_);
    removeMeter(input.meterId);

    const std::string resolvedSourceElement = resolveAudioSourceElement(input.sourceElement);
    auto meter = std::make_unique<RunningAudioMeter>();
    meter->meterId = input.meterId;
    meter->pipeline = gst_pipeline_new(("audio-meter-" + input.meterId).c_str());
    meter->source = gst_element_factory_make(resolvedSourceElement.c_str(), nullptr);
    meter->convert = gst_element_factory_make("audioconvert", nullptr);
    meter->resample = gst_element_factory_make("audioresample", nullptr);
    meter->capsFilter = gst_element_factory_make("capsfilter", nullptr);
    meter->level = gst_element_factory_make("level", nullptr);
    meter->sink = gst_element_factory_make("fakesink", nullptr);
    meter->volume = clampPercent(input.volume);
    meter->balance = clampBalance(input.balance);
    meter->mono = input.mono;
    meter->owner = this;

    if (meter->pipeline == nullptr ||
        meter->source == nullptr ||
        meter->convert == nullptr ||
        meter->resample == nullptr ||
        meter->capsFilter == nullptr ||
        meter->level == nullptr ||
        meter->sink == nullptr) {
        if (meter->pipeline != nullptr) {
            gst_object_unref(meter->pipeline);
        }
        return AudioMeterResult{false, "Failed to create audio meter pipeline elements"};
    }

    setAudioSourceDevice(meter->source, resolvedSourceElement, input.deviceName, input.devicePath);

    GstCaps* rawCaps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "S16LE", nullptr);
    g_object_set(meter->capsFilter, "caps", rawCaps, nullptr);
    gst_caps_unref(rawCaps);

    g_object_set(
        meter->level,
        "interval",
        static_cast<guint64>(50 * GST_MSECOND),
        "post-messages",
        TRUE,
        nullptr
    );
    g_object_set(meter->sink, "sync", FALSE, nullptr);

    gst_bin_add_many(
        GST_BIN(meter->pipeline),
        meter->source,
        meter->convert,
        meter->resample,
        meter->capsFilter,
        meter->level,
        meter->sink,
        nullptr
    );

    if (!gst_element_link_many(
            meter->source,
            meter->convert,
            meter->resample,
            meter->capsFilter,
            meter->level,
            meter->sink,
            nullptr
        )) {
        gst_object_unref(meter->pipeline);
        return AudioMeterResult{false, "Failed to link audio meter pipeline"};
    }

    meter->bus = gst_element_get_bus(meter->pipeline);

    GstPad* capsFilterSrcPad = gst_element_get_static_pad(meter->capsFilter, "src");
    if (capsFilterSrcPad == nullptr) {
        gst_object_unref(meter->bus);
        gst_object_unref(meter->pipeline);
        return AudioMeterResult{false, "Failed to inspect audio meter caps src pad"};
    }

    gst_pad_add_probe(capsFilterSrcPad, GST_PAD_PROBE_TYPE_BUFFER, onAudioMeterBuffer, meter.get(), nullptr);
    gst_object_unref(capsFilterSrcPad);

    const GstStateChangeReturn stateResult = gst_element_set_state(meter->pipeline, GST_STATE_PLAYING);
    if (stateResult == GST_STATE_CHANGE_FAILURE) {
        gst_object_unref(meter->bus);
        gst_object_unref(meter->pipeline);
        return AudioMeterResult{false, "Failed to start audio meter pipeline"};
    }

    GstState currentState = GST_STATE_NULL;
    GstState pendingState = GST_STATE_NULL;
    const GstStateChangeReturn waitResult = gst_element_get_state(
        meter->pipeline,
        &currentState,
        &pendingState,
        kAudioMeterStartTimeout
    );

    if (waitResult == GST_STATE_CHANGE_FAILURE) {
        GstMessage* message = gst_bus_timed_pop_filtered(meter->bus, GST_SECOND, GST_MESSAGE_ERROR);
        std::string failureMessage = "Failed to start audio meter pipeline";

        if (message != nullptr) {
            failureMessage = travis::media_engine::pipeline::readGstErrorMessage(message);
            gst_message_unref(message);
        }

        gst_object_unref(meter->bus);
        gst_object_unref(meter->pipeline);
        return AudioMeterResult{false, failureMessage};
    }

    if (waitResult == GST_STATE_CHANGE_ASYNC) {
        gst_element_set_state(meter->pipeline, GST_STATE_NULL);
        gst_element_get_state(meter->pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
        gst_object_unref(meter->bus);
        gst_object_unref(meter->pipeline);
        return AudioMeterResult{false, "Timed out starting audio meter pipeline"};
    }

    meter->busThread = std::thread(pollAudioMeterBus, meter.get());
    meters_[input.meterId] = std::move(meter);
    return AudioMeterResult{true, "Audio meter started"};
}

AudioMeterResult AudioMeterEngine::setSettings(
    const std::string& meterId,
    double volume,
    bool mono,
    double balance
) {
    std::lock_guard<std::mutex> lock(metersMutex_);
    const auto meter = meters_.find(meterId);

    if (meter == meters_.end()) {
        return AudioMeterResult{true, "Audio meter is not running"};
    }

    {
        std::lock_guard<std::mutex> settingsLock(meter->second->settingsMutex);
        meter->second->volume = clampPercent(volume);
        meter->second->balance = clampBalance(balance);
        meter->second->mono = mono;
    }

    return AudioMeterResult{true, "Audio meter settings updated"};
}

AudioMeterResult AudioMeterEngine::stopMeter(const std::string& meterId) {
    if (meterId.empty()) {
        return AudioMeterResult{false, "meterId is required"};
    }

    std::lock_guard<std::mutex> lock(metersMutex_);
    removeMeter(meterId);
    return AudioMeterResult{true, "Audio meter stopped"};
}

void AudioMeterEngine::stopAll() {
    std::lock_guard<std::mutex> lock(metersMutex_);
    while (!meters_.empty()) {
        removeMeter(meters_.begin()->first);
    }
}

void AudioMeterEngine::emitLevel(const AudioMeterLevel& level) {
    LevelCallback callback;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback = levelCallback_;
    }

    if (callback) {
        callback(level);
    }
}

void AudioMeterEngine::removeMeter(const std::string& meterId) {
    const auto meter = meters_.find(meterId);
    if (meter == meters_.end()) {
        return;
    }

    meter->second->stopRequested.store(true);

    if (meter->second->busThread.joinable()) {
        meter->second->busThread.join();
    }

    gst_element_set_state(meter->second->pipeline, GST_STATE_NULL);
    gst_element_get_state(meter->second->pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
    gst_object_unref(meter->second->bus);
    gst_object_unref(meter->second->pipeline);
    meters_.erase(meter);
}

} // namespace travis::media_engine::metering
