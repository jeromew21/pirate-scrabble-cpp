#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

enum class SocketMessageType {
    Message, Open, Close, Error
};

struct SocketCloseInfo {
    uint16_t code;
    std::string reason;
    //bool remote;
};

struct SocketErrorInfo {
    //uint32_t retries = 0;
    //double wait_time = 0;
    //int http_status = 0;
    std::string reason;
    //bool decompressionError = false;
};

struct SocketMessage {
    // todo: make union on type
    SocketMessageType type;
    SocketCloseInfo closeInfo;
    SocketErrorInfo errorInfo;
    std::string str;
};

using SocketMessageCallback = std::function<void(const SocketMessage &message)>;

void SocketPlatformInit();

class Socket {
public:
    Socket(); // defined in .cpp
    ~Socket(); // must be defined where Impl is complete

    Socket(Socket &&) noexcept; // movable
    Socket &operator=(Socket &&) noexcept;

    Socket(const Socket &) = delete;

    Socket &operator=(const Socket &) = delete;

    void SetUrl(const std::string &url) const;

    void DisableAutomaticReconnection() const;

    void SetOnMessageCallback(const SocketMessageCallback &callback) const;

    void Send(const std::string &message) const;

    void Start() const;

    void Stop() const;

    void Close() const;

private:
    struct Impl; // forward declaration only
    std::unique_ptr<Impl> impl_;
};
