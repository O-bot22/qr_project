#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <algorithm>
constexpr uint32_t MAX_IMAGE = 4 * 1024 * 1024;
using Clock = std::chrono::steady_clock;
inline volatile sig_atomic_t *io_cancel = nullptr;
// Exact transfers: TCP may split a header or payload at any byte boundary.
// Return 1=complete, 0=EOF, -1=system error, -2=inactivity timeout.
inline int transfer(int fd, void *buffer, size_t length, bool sending, int seconds, volatile sig_atomic_t *cancel=io_cancel) {
    auto *p = static_cast<unsigned char *>(buffer);
    auto deadline = Clock::now() + std::chrono::seconds(seconds);
    while (length) {
        if (cancel && *cancel) return -1;
        auto left = std::chrono::duration_cast<std::chrono::milliseconds>(deadline-Clock::now()).count();
        if (left <= 0) return -2;
        pollfd item{fd, static_cast<short>(sending ? POLLOUT : POLLIN), 0};
        int ready = poll(&item, 1, static_cast<int>(std::min<int64_t>(left, 1000)));
        if (ready < 0) { if (errno == EINTR) { continue; } return -1; }
        if (!ready) continue;
        ssize_t n = sending ? send(fd, p, length, MSG_NOSIGNAL | MSG_DONTWAIT)
                            : recv(fd, p, length, MSG_DONTWAIT);
        if (n < 0) {
            if (errno == EINTR) { continue; }
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return -1;
        }
        if (!n) return 0;
        p += n; length -= static_cast<size_t>(n);
        deadline = Clock::now() + std::chrono::seconds(seconds);
    }
    return 1;
}
inline bool reply(int fd, uint32_t code, const std::string &message, int timeout) {
    uint32_t header[2]{htonl(code), htonl(static_cast<uint32_t>(message.size()))};
    return transfer(fd, header, sizeof(header), true, timeout) == 1 &&
        transfer(fd, const_cast<char *>(message.data()), message.size(), true, timeout) == 1;
}
inline int number(const char *s, int low, int high) {
    std::string value(s); size_t end = 0;
    long n = std::stol(value, &end);
    if (end != value.size() || n < low || n > high) throw std::runtime_error("invalid numeric argument: " + value);
    return static_cast<int>(n);
}
