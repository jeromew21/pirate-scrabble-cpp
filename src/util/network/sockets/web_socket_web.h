#pragma once

#include "web_socket.h"

#include <emscripten/websocket.h>
#include <string>

struct WebSocketWeb : WebSocketImpl {
    EMSCRIPTEN_WEBSOCKET_T ws{};
    std::string url;

    explicit WebSocketWeb(const std::string& url_);

    void connect() override;

    void send(const std::string& message) override;

    void close() override;
};
