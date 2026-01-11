#pragma once
#include <functional>
#include <string>
#include <memory>

struct IWebSocket {
    using Ptr = std::shared_ptr<IWebSocket>;

    virtual ~IWebSocket() = default;

    virtual void connect() = 0;
    virtual void send(const std::string& message) = 0;
    virtual void close() = 0;

    // Event callbacks
    std::function<void()> on_open;
    std::function<void(const std::string&)> on_message;
    std::function<void()> on_close;
    std::function<void(const std::string&)> on_error;
};
