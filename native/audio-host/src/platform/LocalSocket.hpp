#pragma once
// Authenticated loopback transport shared by the Windows and Linux hosts.
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "CommandReader.hpp"

namespace easyknack::platform {
/** Start the platform networking runtime. No-op on POSIX. */
inline void initializeNetworking() {
#ifdef _WIN32
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data)) throw std::runtime_error("Winsock initialization failed");
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

inline void shutdownNetworking() noexcept {
#ifdef _WIN32
    WSACleanup();
#endif
}

/** One IPv4 loopback listener or accepted connection. */
class LocalSocket {
public:
#ifdef _WIN32
    using Handle = SOCKET;
    using Length = int;
    static constexpr Handle invalid = INVALID_SOCKET;
#else
    using Handle = int;
    using Length = socklen_t;
    static constexpr Handle invalid = -1;
#endif

    LocalSocket() = default;
    ~LocalSocket() { close(); }
    LocalSocket(const LocalSocket&) = delete;
    LocalSocket& operator=(const LocalSocket&) = delete;
    LocalSocket(LocalSocket&& other) noexcept : value(other.value) { other.value = invalid; }
    LocalSocket& operator=(LocalSocket&& other) noexcept {
        if (this != &other) { close(); value = other.value; other.value = invalid; }
        return *this;
    }

    void listen() {
        close();
        value = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (value == invalid) throw std::runtime_error("socket failed");
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (::bind(value, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0
                || ::listen(value, 1) != 0)
            throw std::runtime_error("loopback bind failed");
    }

    std::uint16_t port() const {
        sockaddr_in address{};
        Length length = sizeof(address);
        if (getsockname(value, reinterpret_cast<sockaddr*>(&address), &length) != 0)
            throw std::runtime_error("port query failed");
        return ntohs(address.sin_port);
    }

    LocalSocket accept(std::chrono::milliseconds timeout) {
        if (!waitReadable(timeout)) throw std::runtime_error("client startup timeout");
        for (;;) {
            Handle client = ::accept(value, nullptr, nullptr);
            if (client != invalid) return LocalSocket(client);
#ifndef _WIN32
            if (errno == EINTR) continue;
#endif
            throw std::runtime_error("accept failed");
        }
    }

    /** Disable Nagle and bound each blocking receive/send. */
    void configure() {
#ifdef _WIN32
        BOOL noDelay = TRUE;
        setsockopt(value, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&noDelay), sizeof(noDelay));
        DWORD deadline = 5000;
        setsockopt(value, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&deadline), sizeof(deadline));
        setsockopt(value, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&deadline), sizeof(deadline));
#else
        int noDelay = 1;
        setsockopt(value, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
        timeval deadline{5, 0};
        setsockopt(value, SOL_SOCKET, SO_RCVTIMEO, &deadline, sizeof(deadline));
        setsockopt(value, SOL_SOCKET, SO_SNDTIMEO, &deadline, sizeof(deadline));
#endif
    }

    std::string readLine(easyknack::CommandReader& reader) {
        return reader.read([this](char* buffer, std::size_t size) {
#ifdef _WIN32
            return static_cast<int>(recv(value, buffer, static_cast<int>(size), 0));
#else
            ssize_t count;
            do { count = recv(value, buffer, size, 0); } while (count < 0 && errno == EINTR);
            return static_cast<int>(count);
#endif
        });
    }

    void writeLine(const std::string& line) {
        std::string message = line + "\n";
        std::size_t offset = 0;
        while (offset < message.size()) {
#ifdef _WIN32
            int sent = send(value, message.data() + offset,
                    static_cast<int>(message.size() - offset), 0);
#else
#ifdef MSG_NOSIGNAL
            constexpr int flags = MSG_NOSIGNAL;
#else
            constexpr int flags = 0;
#endif
            auto sent = send(value, message.data() + offset, message.size() - offset, flags);
            if (sent < 0 && errno == EINTR) continue;
#endif
            if (sent <= 0) throw std::runtime_error("connection closed");
            offset += static_cast<std::size_t>(sent);
        }
    }

    void close() noexcept {
        if (value == invalid) return;
#ifdef _WIN32
        closesocket(value);
#else
        ::close(value);
#endif
        value = invalid;
    }

private:
    explicit LocalSocket(Handle handle) : value(handle) {}

    bool waitReadable(std::chrono::milliseconds timeout) const {
#ifdef _WIN32
        fd_set ready;
        FD_ZERO(&ready);
        FD_SET(value, &ready);
        timeval wait{static_cast<long>(timeout.count() / 1000),
                     static_cast<long>((timeout.count() % 1000) * 1000)};
        return select(0, &ready, nullptr, nullptr, &wait) == 1;
#else
        pollfd descriptor{value, POLLIN, 0};
        for (;;) {
            int result = poll(&descriptor, 1, static_cast<int>(timeout.count()));
            if (result < 0 && errno == EINTR) continue;
            return result == 1;
        }
#endif
    }

    Handle value = invalid;
};
} // namespace easyknack::platform
