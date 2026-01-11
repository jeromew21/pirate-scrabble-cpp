#pragma once

#include "web_socket.h"

#include <ixwebsocket/IXWebSocket.h>

struct WebSocketDesktop : public IWebSocket {
    ix::WebSocket ws;
    std::string url;

    WebSocketDesktop(const std::string& url_) : url(url_) {}

    void connect() override {
        ws.setUrl(url);

        ws.disableAutomaticReconnection();

        ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
            switch(msg->type) {
                case ix::WebSocketMessageType::Open:
                    if (on_open) on_open();
                    break;
                case ix::WebSocketMessageType::Message:
                    if (on_message) on_message(msg->str);
                    break;
                case ix::WebSocketMessageType::Close:
                    if (on_close) on_close();
                    break;
                case ix::WebSocketMessageType::Error:
                    if (on_error) on_error(msg->errorInfo.reason);
                    break;
                default: break;
            }
        });

        ws.start();
    }

    void send(const std::string& message) override {
        ws.send(message);
    }

    void close() override {
        ws.stop();
    }
};
