#pragma once

#include "web_socket.h"
#include <emscripten/websocket.h>
#include <string>

struct WebSocketWeb : public IWebSocket {
    EMSCRIPTEN_WEBSOCKET_T ws;
    std::string url;

    WebSocketWeb(const std::string& url_) : url(url_), ws(0) {}

    void connect() override {
        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);
        attr.url = url.c_str();
        attr.createOnMainThread = EM_TRUE;

        ws = emscripten_websocket_new(&attr);

        emscripten_websocket_set_onopen_callback(ws, this,
            [](int, const EmscriptenWebSocketOpenEvent*, void* user) -> EM_BOOL {
                auto self = static_cast<WebSocketWeb*>(user);
                if (self->on_open) self->on_open();
                return EM_TRUE;
            });

        emscripten_websocket_set_onmessage_callback(ws, this,
            [](int, const EmscriptenWebSocketMessageEvent* e, void* user) -> EM_BOOL {
                auto self = static_cast<WebSocketWeb*>(user);
                if (self->on_message) self->on_message(std::string((char*)e->data, e->numBytes));
                return EM_TRUE;
            });

        emscripten_websocket_set_onerror_callback(ws, this,
            [](int, const EmscriptenWebSocketMessageEvent*, void* user) -> EM_BOOL {
                auto self = static_cast<WebSocketWeb*>(user);
                if (self->on_error) self->on_error("WebSocket error");
                return EM_TRUE;
            });

        emscripten_websocket_set_onclose_callback(ws, this,
            [](int, const EmscriptenWebSocketCloseEvent*, void* user) -> EM_BOOL {
                auto self = static_cast<WebSocketWeb*>(user);
                if (self->on_close) self->on_close();
                return EM_TRUE;
            });
    }

    void send(const std::string& message) override {
        emscripten_websocket_send_utf8_text(ws, message.c_str());
    }

    void close() override {
        emscripten_websocket_close(ws, 1000, "Closed by client");
    }
};
