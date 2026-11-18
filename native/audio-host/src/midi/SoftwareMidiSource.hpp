#pragma once
// Keyboard-only MIDI source for the minimal Linux port. Hardware MIDI input is
// not opened; the on-screen keyboard and Java-side MIDI drivers keep working.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include "MidiQueue.hpp"

namespace easyknack {
class SoftwareMidiSource : public MidiSource {
public:
    struct Device { std::uint64_t key; unsigned index; std::string name; };
private:
    struct Slot {
        std::atomic<std::uint64_t> key{0};
        std::atomic<bool> driverEnabled{false};
        MidiQueue queue, driverQueue;
        MidiBlock block;
    };
    Slot keyboard;
    std::atomic<bool> running{false};
    std::vector<std::uint64_t> driverWanted;
    bool learningDrivers = false;
    std::string problem;
    static std::int64_t now() noexcept {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
    }
public:
    explicit SoftwareMidiSource(bool /*useHardware*/ = true) {
        keyboard.key.store(1);
    }
    std::vector<Device> enumerate() const { return {}; }
    void prepare(std::vector<std::uint64_t> /*devices*/) {}
    void prepareDrivers(std::vector<std::uint64_t> devices, bool learning) {
        std::sort(devices.begin(), devices.end());
        devices.erase(std::unique(devices.begin(), devices.end()), devices.end());
        if (driverWanted == devices && learningDrivers == learning) return;
        driverWanted = std::move(devices);
        learningDrivers = learning;
        keyboard.driverEnabled.store(learning
                || std::find(driverWanted.begin(), driverWanted.end(), 1) != driverWanted.end());
    }
    void refresh() {}
    std::vector<std::pair<std::uint64_t, std::uint32_t>> driverEvents() {
        std::vector<std::pair<std::uint64_t, std::uint32_t>> result;
        auto end = now() + 1;
        MidiBlock block;
        keyboard.driverQueue.drain(block, 0, end, 1);
        if (block.panic) result.emplace_back(keyboard.key.load(), 255);
        for (std::size_t i = 0; i < block.count; ++i)
            result.emplace_back(keyboard.key.load(), block.events[i].message);
        return result;
    }
    void setRunning(bool value) noexcept {
        running.store(value, std::memory_order_release);
        if (!value) panic();
    }
    void panic() noexcept {
        keyboard.queue.panic();
        keyboard.driverQueue.panic();
    }
    void keyboardEvent(std::uint32_t message) noexcept {
        if (message == 255) {
            keyboard.queue.panic();
            keyboard.driverQueue.panic();
            return;
        }
        if (keyboard.driverEnabled.load()) keyboard.driverQueue.push(now(), message);
        if (running.load(std::memory_order_acquire)) keyboard.queue.push(now(), message);
    }
    const std::string& status() const { return problem; }
    void beginBlock(std::size_t frames, double rate) noexcept override {
        auto end = now();
        auto start = end - static_cast<std::int64_t>(frames*1e9/rate);
        keyboard.queue.drain(keyboard.block, start, end, frames);
        keyboard.block.source = 1;
        keyboard.block.generation = 1;
        if (!running.load(std::memory_order_acquire)) {
            keyboard.block.clear();
            keyboard.block.panic = true;
        }
    }
    const MidiBlock* events(std::uint64_t key) const noexcept override {
        if (key == 1) return &keyboard.block;
        return nullptr;
    }
};
} // namespace easyknack
