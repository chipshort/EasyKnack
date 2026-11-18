#pragma once
// Platform-specific audio, volume and MIDI implementations for the host.
#if defined(_WIN32)
#include "../audio/RoutingAudioBackend.hpp"
#include "../audio/WindowsDeviceVolume.hpp"
#include "../midi/WindowsMidiSource.hpp"

namespace easyknack::platform {
using AudioBackendImpl = easyknack::RoutingAudioBackend;
using DeviceVolumeImpl = easyknack::WindowsDeviceVolume;
using MidiSourceImpl = easyknack::WindowsMidiSource;
} // namespace easyknack::platform
#else
#include "../audio/LinuxAudioBackend.hpp"
#include "../audio/NullDeviceVolume.hpp"
#include "../midi/SoftwareMidiSource.hpp"

namespace easyknack::platform {
using AudioBackendImpl = easyknack::LinuxAudioBackend;
using DeviceVolumeImpl = easyknack::NullDeviceVolume;
using MidiSourceImpl = easyknack::SoftwareMidiSource;
} // namespace easyknack::platform
#endif
