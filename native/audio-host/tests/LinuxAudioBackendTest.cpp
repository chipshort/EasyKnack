#define MA_NO_FLAC
#define MA_NO_ENCODING
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include "../src/audio/LinuxAudioBackend.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

int main() {
    using namespace easyknack;
    try {
        AudioEngine engine;
        LinuxAudioBackend backend(engine, true);
        auto outputs = backend.enumerateOutputs();
        bool hasDefault = false;
        for (const auto& endpoint : outputs) if (endpoint.id == "default") hasDefault = true;
        if (!hasDefault) throw std::runtime_error("default output endpoint is missing");
        auto inputs = backend.enumerateInputs();
        bool hasDefaultInput = false;
        for (const auto& endpoint : inputs) if (endpoint.id == "default") hasDefaultInput = true;
        if (!hasDefaultInput) throw std::runtime_error("default input endpoint is missing");

        backend.prepare({{"default", false, AudioCaptureMode::Copy}});
        if (!backend.status().empty()) throw std::runtime_error("unexpected prepare status: " + backend.status());
        auto before = backend.statistics();
        backend.setRunning(true);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (backend.statistics().playedFrames == before.playedFrames
                && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto after = backend.statistics();
        if (after.playedFrames <= before.playedFrames)
            throw std::runtime_error("null playback clock did not advance");
        auto generation = backend.generation();
        backend.setRunning(false);
        if (backend.generation() == generation) throw std::runtime_error("stop did not update the diagnostics generation");
        if (!backend.status().empty()) throw std::runtime_error("unexpected stop status: " + backend.status());
        std::cout << "Linux null backend prepared, ran and stopped cleanly\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
