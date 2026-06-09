#pragma once

#include <gst/gst.h>

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// Owns lightweight GStreamer audio meter pipelines for live AV dock level feedback.

namespace travis::media_engine::metering {

class AudioMeterEngine;

struct AudioMeterResult {
    bool ok = false;
    std::string message;
};

struct AudioMeterLevel {
    std::string meterId;
    int leftLevel = 0;
    int rightLevel = 0;
    double leftPeakDb = -60.0;
    double rightPeakDb = -60.0;
};

struct AudioMeterInput {
    std::string meterId;
    std::string deviceName;
    std::string devicePath;
    std::string sourceElement;
    double volume = 100.0;
    bool mono = false;
    double balance = 0.0;
};

struct RunningAudioMeter {
    std::string meterId;
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* convert = nullptr;
    GstElement* resample = nullptr;
    GstElement* capsFilter = nullptr;
    GstElement* level = nullptr;
    GstElement* sink = nullptr;
    GstBus* bus = nullptr;
    GstClockTime lastProbeEmitTime = GST_CLOCK_TIME_NONE;
    double volume = 100.0;
    double balance = 0.0;
    bool mono = false;
    std::mutex settingsMutex;
    std::atomic<bool> stopRequested = false;
    std::thread busThread;
    AudioMeterEngine* owner = nullptr;
};

class AudioMeterEngine {
public:
    using LevelCallback = std::function<void(const AudioMeterLevel&)>;

    AudioMeterEngine();
    ~AudioMeterEngine();

    AudioMeterEngine(const AudioMeterEngine&) = delete;
    AudioMeterEngine& operator=(const AudioMeterEngine&) = delete;

    void setLevelCallback(LevelCallback callback);
    [[nodiscard]] AudioMeterResult startMeter(const AudioMeterInput& input);
    [[nodiscard]] AudioMeterResult setSettings(
        const std::string& meterId,
        double volume,
        bool mono,
        double balance
    );
    [[nodiscard]] AudioMeterResult stopMeter(const std::string& meterId);
    void stopAll();

    // Called from GStreamer streaming callbacks; consumers should use setLevelCallback instead.
    void emitLevel(const AudioMeterLevel& level);

private:
    void removeMeter(const std::string& meterId);

    std::map<std::string, std::unique_ptr<RunningAudioMeter>> meters_;
    std::mutex metersMutex_;
    std::mutex callbackMutex_;
    LevelCallback levelCallback_;
};

} // namespace travis::media_engine::metering
