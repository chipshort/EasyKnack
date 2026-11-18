#pragma once
// Endpoint volume is a Windows-only feature; the Linux host keeps the protocol
// shape but reports every operation as unavailable.
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace easyknack {
class NullDeviceVolume {
public:
    struct Value { bool available=false; float scalar=0; bool muted=false; };

    explicit NullDeviceVolume(bool /*realDevices*/ = false) {}
    Value get(const std::string& /*id*/) { return {}; }
    bool set(const std::string& /*id*/, float value) {
        if (!std::isfinite(value) || value < 0 || value > 1)
            throw std::runtime_error("volume outside 0..1");
        return false;
    }
    void links(std::set<std::string> ids) {
        if (ids.size() > 32) throw std::runtime_error("too many volume targets");
    }
    void poll() {}
    std::string keyEvent() const { return "0 0 0"; }
    bool keysActive() const { return false; }
};
} // namespace easyknack
