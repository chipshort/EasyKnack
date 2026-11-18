#pragma once
// Linux audio transport. Only this backend knows ALSA/miniaudio devices.
#include "AudioBackend.hpp"
#include "AudioRenderAhead.hpp"
#include "ProtocolText.hpp"
#include "../runtime/GraphRuntime.hpp"
#include "miniaudio.h"
#include <map>
#include <set>
#include <chrono>

namespace easyknack {
/** ALSA-through-miniaudio backend for the minimal Linux port.
 *
 * The engine, ring buffers and protocol are shared with the Windows backend.
 * Windows process capture, ASIO, application routing and endpoint volume are
 * intentionally absent; the host reports them as unavailable. */
class LinuxAudioBackend final : public AudioBackend {
#ifdef EASYKNACK_AUDIO_TEST
    friend struct LinuxAudioBackendTest;
#endif
    struct DeviceInfo { AudioEndpoint endpoint; ma_device_id id{}; };
    struct Stream {
        LinuxAudioBackend* owner = nullptr;
        std::string id, name;
        bool input = false, initialized = false, started = false, drivesDsp = false;
        ma_result openResult = MA_SUCCESS;
        ma_device device{};
        StereoRing<> ring;
        std::unique_ptr<StereoRing<131072>> playback;
        std::array<float, blockSize*2> block{};
        std::atomic<bool> failed{false};
        std::atomic<std::uint64_t> directFrames{0};
        std::atomic<unsigned> maxCapturePacket{0};
        std::atomic<float> peak{0};
        static_assert(std::atomic<float>::is_always_lock_free);
        ~Stream() { if (initialized) ma_device_uninit(&device); }
        bool start() { started = ma_device_start(&device) == MA_SUCCESS; return started; }
        void stop() {
            if (!started) return;
            ma_device_stop(&device);
            started = false;
        }
    };
    AudioEngine& engine;
    ma_context context{}, clockContext{};
    ma_device clock{};
    bool contextReady = false, clockContextReady = false, clockReady = false;
    bool nullBackend = false, running = false;
    bool clockEnabled = true;
    AudioRenderAhead renderAhead;
    Stream* master = nullptr; // Changed only while every callback is stopped.
    float* directOutput = nullptr; // DSP thread only, valid for one master callback.
    std::size_t directOffset = 0;
    std::uint64_t diagnosticGeneration = 1;
    static constexpr std::size_t inputReserve = 256;
    static std::size_t inputTarget(const Stream& stream) noexcept {
        return std::min<std::size_t>(4096, std::max<std::size_t>(inputReserve,
                stream.maxCapturePacket.load(std::memory_order_relaxed)*2));
    }
    std::vector<DeviceInfo> inputs, outputs;
    std::vector<std::unique_ptr<Stream>> streams;
    std::vector<AudioRoute> selectedRoutes;
    std::string preparedKey;
    static void deviceCallback(ma_device* device, void* output, const void* input, ma_uint32 frames) {
        auto& stream = *static_cast<Stream*>(device->pUserData);
        if (stream.input) {
            stream.maxCapturePacket.store(std::max(stream.maxCapturePacket.load(std::memory_order_relaxed), frames),
                    std::memory_order_relaxed);
            stream.ring.write(static_cast<const float*>(input), frames);
        } else if (stream.drivesDsp) {
            auto& self = *stream.owner;
            if (self.renderAhead.active()) {
                self.renderAhead.read(static_cast<float*>(output), frames);
                stream.directFrames.fetch_add(frames, std::memory_order_relaxed);
                return;
            }
            self.directOutput = static_cast<float*>(output);
            self.directOffset = 0;
            self.engine.render(self.directOutput, frames, &self, true);
            self.directOutput = nullptr;
            if (RenderCadence::period > 1) self.renderAhead.enable(RenderCadence::period);
            stream.directFrames.fetch_add(frames, std::memory_order_relaxed);
        } else if (stream.playback) {
            stream.playback->readAdaptive(static_cast<float*>(output), frames);
        } else {
            stream.ring.readAdaptive(static_cast<float*>(output), frames);
        }
    }
    static unsigned renderQueued(void* context, float* output, unsigned frames) noexcept {
        auto& self = *static_cast<LinuxAudioBackend*>(context);
        self.directOutput = output;
        self.directOffset = 0;
        self.engine.render(output, frames, &self, true);
        self.directOutput = nullptr;
        return RenderCadence::period;
    }
    static void notification(const ma_device_notification* event) {
        if (event->type == ma_device_notification_type_stopped)
            static_cast<Stream*>(event->pDevice->pUserData)->failed.store(true, std::memory_order_relaxed);
    }
    static void clockCallback(ma_device* device, void* output, const void*, ma_uint32 frames) {
        auto& self = *static_cast<LinuxAudioBackend*>(device->pUserData);
        self.engine.render(static_cast<float*>(output), frames, &self, true);
    }
    void discover() {
        inputs.clear();
        outputs.clear();
        // Stable logical defaults follow the OS; explicit hardware IDs never fall back.
        outputs.push_back({{"default", "System Default", AudioEndpointType::HardwareOutput}});
        inputs.push_back({{"default", "System Default", AudioEndpointType::HardwareInput}});
        ma_device_info *playback = nullptr, *capture = nullptr;
        ma_uint32 playbackCount = 0, captureCount = 0;
        if (ma_context_get_devices(&context, &playback, &playbackCount, &capture, &captureCount) == MA_SUCCESS) {
            std::map<std::string, unsigned> occurrences;
            auto add = [&](const ma_device_info& device, bool input) {
                std::string identity = nullBackend
                        ? std::to_string(&device - (input ? capture : playback))
                        : std::string(device.id.alsa);
                std::string name = device.name;
                auto ordinal = occurrences[(input ? "i:" : "o:") + identity]++;
                std::string id = (input ? "in:" : "out:") + identity
                        + (ordinal ? "#" + std::to_string(ordinal + 1) : "");
                (input ? inputs : outputs).push_back({{id, name,
                        input ? AudioEndpointType::HardwareInput : AudioEndpointType::HardwareOutput}, device.id});
            };
            for (ma_uint32 i = 0; i < playbackCount; ++i) add(playback[i], false);
            for (ma_uint32 i = 0; i < captureCount; ++i) add(capture[i], true);
        }
    }
    const DeviceInfo* find(const std::string& id, bool input) const {
        const auto& list = input ? inputs : outputs;
        for (const auto& item : list) if (item.endpoint.id == id) return &item;
        return nullptr;
    }
    std::string routeKey(const std::vector<AudioRoute>& routes) const {
        std::map<std::pair<bool, std::string>, AudioRoute> unique;
        for (const auto& route : routes) unique[{route.input, route.endpoint}] = route;
        std::string key;
        for (const auto& entry : unique) {
            const auto& route = entry.second;
            key += (route.input ? "i" : "o") + hexText(route.endpoint) + ";";
        }
        return key;
    }
    static void prime(Stream& stream) {
        stream.ring.reset();
        if (stream.playback) stream.playback->reset();
        stream.directFrames.store(0);
        if (!stream.input && stream.playback) stream.playback->write(nullptr, StereoRing<>::adaptiveTargetFrames);
    }
    void pauseStreams() {
        ma_device_stop(&clock);
        // The DSP producer must finish before any consumer/buffer is reset or destroyed.
        if (master) master->stop();
        renderAhead.stop();
        for (auto& stream : streams) if (stream->initialized && stream.get() != master) stream->stop();
        master = nullptr;
    }
    void resumeStreams() {
        if (running) {
            if (clockEnabled) for (auto& stream : streams)
                if (!stream->input && stream->initialized) { master = stream.get(); break; }
            for (auto& stream : streams) {
                stream->drivesDsp = stream.get() == master;
                prime(*stream);
                stream->failed.store(false);
            }
            // Capture and secondary sinks are ready before the master starts the DSP.
            for (auto& stream : streams) if (stream->initialized && stream.get() != master)
                stream->failed.store(!stream->start());
            if (master) renderAhead.start(renderQueued, this);
            if (master && !master->start()) {
                renderAhead.stop();
                master->failed.store(true);
                master = nullptr;
            }
        }
        if (clockEnabled && !master && ma_device_start(&clock) != MA_SUCCESS)
            throw std::runtime_error("audio fallback clock start failed");
    }
    AudioEndpoint withAvailability(AudioEndpoint endpoint) const {
        for (const auto& stream : streams) if (stream->id == endpoint.id && stream->input ==
                (endpoint.type == AudioEndpointType::HardwareInput))
            if (!stream->initialized || stream->failed.load()) endpoint.available = false;
        return endpoint;
    }
    void release() noexcept {
        if (clockReady) pauseStreams();
        if (clockReady) { ma_device_uninit(&clock); clockReady = false; }
        streams.clear();
        if (clockContextReady) { ma_context_uninit(&clockContext); clockContextReady = false; }
        if (contextReady) { ma_context_uninit(&context); contextReady = false; }
    }
public:
    float takePeak(const std::string& id) noexcept {
        float peak = 0;
        for (auto& stream : streams)
            if (!stream->input && stream->id == id) peak = std::max(peak, stream->peak.exchange(0));
        return peak;
    }
    std::string bufferDescription() const {
        std::string result;
        for (const auto& stream : streams) if (stream->input) {
            if (!result.empty()) result += " ";
            result += hexText(stream->id) + " " + std::to_string(stream->maxCapturePacket.load())
                    + " " + std::to_string(stream->ring.available()) + " " + std::to_string(inputTarget(*stream));
        }
        return result.empty() ? "-" : result;
    }
    std::string clockDescription() const {
        if (!clockEnabled) return "external";
        if (!master) return "timer";
        return "output " + hexText(master->id) + " "
                + std::to_string(master->device.playback.internalPeriodSizeInFrames) + " "
                + std::to_string(master->device.playback.internalSampleRate) + " "
                + std::to_string(inputReserve) + " render-reserve " + std::to_string(renderAhead.latencyFrames());
    }
    std::uint64_t generation() const { return diagnosticGeneration; }
    bool needsRefresh() const {
        return routeKey(selectedRoutes) != preparedKey || std::any_of(streams.begin(), streams.end(), [this](const auto& s) {
            return find(s->id, s->input) && (!s->initialized || s->failed.load());
        });
    }
    explicit LinuxAudioBackend(AudioEngine& target, bool useNull) : engine(target), nullBackend(useNull) {
        try {
            ma_backend backend = useNull ? ma_backend_null : ma_backend_alsa;
            if (ma_context_init(&backend, 1, nullptr, &context) != MA_SUCCESS)
                throw std::runtime_error("ALSA initialization failed");
            contextReady = true;
            ma_backend timerBackend = ma_backend_null;
            if (ma_context_init(&timerBackend, 1, nullptr, &clockContext) != MA_SUCCESS)
                throw std::runtime_error("audio clock initialization failed");
            clockContextReady = true;
            auto config = ma_device_config_init(ma_device_type_playback);
            config.playback.format = ma_format_f32;
            config.playback.channels = 2;
            config.sampleRate = sampleRate;
            config.periodSizeInFrames = blockSize;
            config.dataCallback = clockCallback;
            config.pUserData = this;
            if (ma_device_init(&clockContext, &config, &clock) != MA_SUCCESS)
                throw std::runtime_error("audio clock failed");
            clockReady = true;
            discover();
            if (ma_device_start(&clock) != MA_SUCCESS) throw std::runtime_error("audio clock start failed");
        } catch (...) { release(); throw; }
    }
    ~LinuxAudioBackend() override { release(); }
    AudioBackendCapabilities capabilities() const override { return {false, false, false, false, false}; }
    std::vector<AudioEndpoint> enumerateInputs() override {
        discover();
        std::vector<AudioEndpoint> result;
        for (const auto& input : inputs) result.push_back(withAvailability(input.endpoint));
        return result;
    }
    std::vector<AudioEndpoint> enumerateOutputs() override {
        std::vector<AudioEndpoint> result;
        for (const auto& output : outputs) result.push_back(withAvailability(output.endpoint));
        return result;
    }
    void prepare(const std::vector<AudioRoute>& routes) override {
        std::map<std::pair<bool, std::string>, AudioRoute> unique;
        for (const auto& route : routes) {
            if (route.mode != AudioCaptureMode::Copy)
                throw std::runtime_error("ROUTE requires a routing backend");
            unique[{route.input, route.endpoint}] = route;
        }
        if (unique.size() > 32) throw std::runtime_error("Maximum 32 simultaneous audio endpoints");
        std::string key = routeKey(routes);
        if (unique.size() > 64) throw std::runtime_error("Maximum 64 simultaneous audio streams");
        if (key == preparedKey) return;
        ++diagnosticGeneration;
        pauseStreams(); // No callback sees a half-open topology.
        streams.clear();
        selectedRoutes = routes;
        preparedKey = key;
        for (const auto& entry : unique) {
            const auto& route = entry.second;
            auto info = find(route.endpoint, route.input);
            auto stream = std::make_unique<Stream>();
            stream->owner = this;
            stream->id = route.endpoint;
            stream->input = route.input;
            if (!route.input) stream->playback = std::make_unique<StereoRing<131072>>();
            stream->name = info ? info->endpoint.name : route.endpoint;
            if (info) {
                auto config = ma_device_config_init(route.input ? ma_device_type_capture : ma_device_type_playback);
                config.sampleRate = sampleRate;
                config.periodSizeInFrames = 128;
                config.noFixedSizedCallback = MA_TRUE;
                config.dataCallback = deviceCallback;
                config.notificationCallback = notification;
                config.pUserData = stream.get();
                config.capture.format = ma_format_f32;
                config.capture.channels = 2;
                config.playback.format = ma_format_f32;
                config.playback.channels = 2;
                if (route.endpoint != "default") {
                    if (route.input) config.capture.pDeviceID = &info->id;
                    else config.playback.pDeviceID = &info->id;
                }
                // miniaudio converts native channel layouts and rates to stereo float/48 kHz here.
                stream->openResult = ma_device_init(&context, &config, &stream->device);
                stream->initialized = stream->openResult == MA_SUCCESS;
            }
            streams.push_back(std::move(stream));
        }
        resumeStreams();
    }
    void refresh() override {
        // Called after enumeration. Missing devices change the key. Failed opens retry.
        if (std::any_of(streams.begin(), streams.end(), [this](const auto& s) {
                return find(s->id, s->input) && (!s->initialized || s->failed.load()); })) preparedKey.clear();
        prepare(selectedRoutes);
    }
    void setRunning(bool value) override {
        if (running == value) return;
        pauseStreams();
        running = value;
        ++diagnosticGeneration;
        if (!value) for (auto& stream : streams) { stream->ring.reset(); stream->failed.store(false); }
        resumeStreams();
    }
    std::string status() const override {
        std::string result;
        for (const auto& stream : streams) if (!stream->initialized || stream->failed.load()) {
            if (!result.empty()) result += "; ";
            result += "Nicht verfügbar: " + stream->name;
            if (stream->openResult != MA_SUCCESS)
                result += " (" + std::string(ma_result_description(stream->openResult)) + ")";
        }
        return result;
    }
    AudioIOStatistics statistics() const override {
        AudioIOStatistics result;
        for (const auto& stream : streams) {
            if (stream->input) result.capturedFrames += stream->ring.writtenFrames();
            else result.playedFrames += stream.get() == master ? stream->directFrames.load()
                    : stream->playback ? stream->playback->readFrames() : stream->ring.readFrames();
            result.underruns += stream->ring.underruns.load();
            result.overruns += stream->ring.overruns.load();
            if (stream->input) {
                result.captureUnder += stream->ring.underruns.load();
                result.captureOver += stream->ring.overruns.load();
            } else {
                result.playbackUnder += stream->ring.underruns.load();
                result.playbackOver += stream->ring.overruns.load();
            }
            result.priming += stream->ring.priming.load();
            if (stream->playback) {
                auto under = stream->playback->underruns.load(), over = stream->playback->overruns.load();
                result.underruns += under;
                result.overruns += over;
                result.playbackUnder += under;
                result.playbackOver += over;
                result.priming += stream->playback->priming.load();
            }
        }
        result.playbackUnder += renderAhead.underruns();
        result.playbackOver += renderAhead.overruns();
        result.underruns += renderAhead.underruns();
        result.overruns += renderAhead.overruns();
        return result;
    }
    void beginBlock(std::size_t frames) noexcept override {
        for (auto& stream : streams) {
            std::fill_n(stream->block.data(), frames*2, 0.f);
            if (stream->input && running && stream->initialized && !stream->failed.load(std::memory_order_relaxed)) {
                stream->ring.readAdaptive(stream->block.data(), frames, inputTarget(*stream));
            }
        }
    }
    float input(const std::string& id, std::size_t channel, std::size_t frame) const noexcept override {
        float value = 0;
        for (const auto& stream : streams)
            if (stream->input && stream->id == id) value += stream->block[2*frame+channel];
        return value;
    }
    void output(const std::string& id, std::size_t channel, const float* samples, std::size_t frames) noexcept override {
        for (auto& stream : streams) if (!stream->input && stream->id == id)
            for (std::size_t i = 0; i < frames; ++i)
                stream->block[2*i+channel] += std::isfinite(samples[i]) ? samples[i] : 0;
    }
    void endBlock(const float* gain, std::size_t frames) noexcept override {
        if (!running) return;
        for (auto& stream : streams) if (!stream->input && stream->initialized
                && !stream->failed.load(std::memory_order_relaxed)) {
            float peak = 0;
            for (std::size_t i = 0; i < frames; ++i) for (unsigned channel = 0; channel < 2; ++channel)
                stream->block[2*i+channel] = std::clamp(stream->block[2*i+channel] * gain[i], -1.f, 1.f);
            for (std::size_t i = 0; i < frames*2; ++i) peak = std::max(peak, std::abs(stream->block[i]));
            stream->peak.store(std::max(peak, stream->peak.load(std::memory_order_relaxed)),
                    std::memory_order_relaxed);
            if (stream.get() == master && directOutput)
                std::copy_n(stream->block.data(), frames*2, directOutput+directOffset*2);
            else if (stream->playback) stream->playback->write(stream->block.data(), frames);
            else stream->ring.write(stream->block.data(), frames);
        }
        directOffset += frames;
    }
};
} // namespace easyknack
