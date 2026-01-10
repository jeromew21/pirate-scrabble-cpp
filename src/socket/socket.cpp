#include "socket.h"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

struct Socket::Impl {
    ix::WebSocket ix_socket;
};

Socket::Socket() : impl_(std::make_unique<Impl>()) {}

Socket::~Socket() = default;

Socket::Socket(Socket&&) noexcept = default;

Socket& Socket::operator=(Socket&&) noexcept = default;

void Socket::SetOnMessageCallback(const SocketMessageCallback &callback) const {
    //messageCallback = callback;
    impl_->ix_socket.setOnMessageCallback([&callback](const ix::WebSocketMessagePtr &msg) {
        SocketMessage socket_message;
        if (msg->type == ix::WebSocketMessageType::Message) {
            socket_message.type = SocketMessageType::Message;
            socket_message.str = msg->str;
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            socket_message.type = SocketMessageType::Open;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            socket_message.type = SocketMessageType::Close;
            socket_message.closeInfo.code = msg->closeInfo.code;
            socket_message.closeInfo.reason = msg->closeInfo.reason;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            socket_message.type = SocketMessageType::Error;
            socket_message.errorInfo.reason = msg->errorInfo.reason;
        }
        callback(socket_message);
    });
}

void Socket::SetUrl(const std::string &url) const {
    impl_->ix_socket.setUrl(url);
}

void Socket::DisableAutomaticReconnection() const {
    impl_->ix_socket.disableAutomaticReconnection();
}

void Socket::Send(const std::string &message) const {
    impl_->ix_socket.send(message);
}

void Socket::Start() const {
    impl_->ix_socket.start();
}

void Socket::Stop() const {
    impl_->ix_socket.stop();
}

void Socket::Close() const {
    impl_->ix_socket.close();
}

void SocketPlatformInit() {
    ix::initNetSystem();
}
