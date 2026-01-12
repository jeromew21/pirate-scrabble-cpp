#include "web_socket_desktop.h"

WebSocketDesktop::WebSocketDesktop(const std::string &url_): url(url_) {}

void WebSocketDesktop::connect() {
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

void WebSocketDesktop::send(const std::string &message) {
    ws.send(message);
}

void WebSocketDesktop::close() {
    ws.stop();
}
