#pragma once

#include "web_socket.h"

#include "ixwebsocket/IXWebSocket.h"

struct WebSocketDesktop : WebSocketImpl {
    ix::WebSocket ws;
    std::string url;

    explicit WebSocketDesktop(const std::string& url_);

    void connect() override;

    void send(const std::string& message) override;

    void close() override;
};
