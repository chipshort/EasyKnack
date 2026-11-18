#pragma once
#include "miniaudio.h"
#include "GaplessMp3Decoder.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace easyknack {
// Separate import process: no device initialization, graph or audio callback.
inline std::filesystem::path audioImportPath(const std::string& hex) {
    if (hex.empty() || hex.size() % 2) throw std::runtime_error("Invalid import path");
    auto digit = [](char c) -> unsigned {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        throw std::runtime_error("Invalid import path encoding");
    };
    std::string text;
    for (size_t i = 0; i < hex.size(); i += 2) {
        char c = static_cast<char>((digit(hex[i]) << 4) | digit(hex[i + 1]));
        if (!c) throw std::runtime_error("Invalid import path");
        text += c;
    }
    return std::filesystem::u8path(text);
}
inline void decodeAudioFile(const std::string& sourceHex, const std::string& outputHex) {
    auto source = audioImportPath(sourceHex), output = audioImportPath(outputHex);
    auto extension = source.extension().u8string();
    for (char& c : extension) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    if (extension != ".wav" && extension != ".mp3") throw std::runtime_error("Only WAV and MP3 files are supported");
    auto config = ma_decoder_config_init(ma_format_f32, 0, 48000);
    ma_decoding_backend_vtable* mp3Backends[]{&GaplessMp3Decoder::backend};
    config.encodingFormat = extension == ".wav" ? ma_encoding_format_wav : ma_encoding_format_unknown;
    if(extension==".mp3"){config.ppCustomBackendVTables=mp3Backends;config.customBackendCount=1;}
    ma_decoder decoder{};
#ifdef _WIN32
    auto opened = ma_decoder_init_file_w(source.c_str(), &config, &decoder);
#else
    auto opened = ma_decoder_init_file(source.c_str(), &config, &decoder);
#endif
    if (opened != MA_SUCCESS)
        throw std::runtime_error("Cannot decode WAV/MP3 file (invalid, unsupported or unreadable audio)");
    struct Cleanup { ma_decoder* value; ~Cleanup() { ma_decoder_uninit(value); } } cleanup{&decoder};
    if(extension==".mp3"&&decoder.pBackendVTable!=&GaplessMp3Decoder::backend)throw std::runtime_error("Cannot decode MP3 with the gapless backend");
    ma_uint32 channels = 0;
    if (ma_decoder_get_data_format(&decoder, nullptr, &channels, nullptr, nullptr, 0) != MA_SUCCESS || channels < 1 || channels > 2)
        throw std::runtime_error("Only mono/stereo audio files are supported");
    std::ofstream file(output, std::ios::binary | std::ios::trunc);
    file.exceptions(std::ios::badbit | std::ios::failbit);
    auto integer = [&](std::uint64_t value, unsigned bytes) {
        for (unsigned i = 0; i < bytes; ++i) file.put(static_cast<char>((value >> (i * 8)) & 255));
    };
    file.write("EKPCM001", 8); integer(48000, 4); integer(channels, 4); integer(0, 8);
    std::array<float, 8192> samples{};
    ma_uint64 total = 0;
    for (;;) {
        ma_uint64 read = 0;
        auto result = ma_decoder_read_pcm_frames(&decoder, samples.data(), samples.size() / channels, &read);
        if (result != MA_SUCCESS && result != MA_AT_END) throw std::runtime_error("Audio decoding failed");
        for (size_t i = 0; i < read * channels; ++i)
            if (!std::isfinite(samples[i])) throw std::runtime_error("Audio contains non-finite samples");
        file.write(reinterpret_cast<const char*>(samples.data()), read * channels * sizeof(float));
        total += read;
        if (read == 0 || result == MA_AT_END) break;
    }
    if (!total) throw std::runtime_error("Empty audio file");
    file.seekp(16); integer(total, 8); file.close();
}
}
