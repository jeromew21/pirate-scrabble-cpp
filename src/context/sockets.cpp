#include "sockets.h"

#include "serialization/types.h"
#include "util/logging/logging.h"

void UserLoginSocket(Queue &recvLoginQueue, const std::string &username, const std::string &password) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/account/login";

    const auto payload = serialize(UserLoginAttempt{username, password});

    const WebSocket::Ptr ws = std::make_shared<WebSocket>(url);

    ws->on_open = [&]() {
        ws->send(payload);
    };
    ws->on_message = [&recvLoginQueue](const std::string& msg) {
        recvLoginQueue.enqueue(msg);
    };
    ws->on_error = [](const std::string& err) {
        Logger::instance().error("Credential auth socket error: {}", err);
    };
    ws->on_close = []() {
        Logger::instance().error("Credential auth socket closed");
    };

    ws->connect();

    // todo refactor to not use thread
    std::this_thread::sleep_for(std::chrono::seconds(4));
}

void TokenAuthSocket(Queue &recvLoginQueue, std::string token) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/account/tokenAuth";
    const WebSocket::Ptr ws = std::make_shared<WebSocket>(url);

    ws->on_open = [&]() {
        ws->send(token);
    };
    ws->on_message = [&recvLoginQueue](const std::string& msg) {
        recvLoginQueue.enqueue(msg);
    };
    ws->on_error = [](const std::string& err) {
        Logger::instance().error("Token auth socket error: {}", err);
    };
    ws->on_close = []() {
        Logger::instance().error("Token auth socket closed");
    };

    ws->connect();

    // todo refactor to not use thread
    std::this_thread::sleep_for(std::chrono::seconds(4));
}

void NewGameSocket(Queue &recvLoginQueue, std::string token) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/multiplayer/create";
    const WebSocket::Ptr ws = std::make_shared<WebSocket>(url);

    ws->on_open = [&]() {
        ws->send(token);
    };
    ws->on_message = [&recvLoginQueue](const std::string& msg) {
        recvLoginQueue.enqueue(msg);
    };
    ws->on_error = [](const std::string& err) {
        Logger::instance().error("New game socket error: {}", err);
    };
    ws->on_close = []() {
        Logger::instance().error("New game socket closed");
    };

    ws->connect();

    // todo refactor to not use thread
    std::this_thread::sleep_for(std::chrono::seconds(4));
}

WebSocketImpl *create_multiplayer_game_socket(Queue *recvLoginQueue, const std::string& token, const std::string &game_id) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/multiplayer/v2/" + game_id;
    auto *ws = new WebSocket(url);

    ws->on_open = [ws, token]() {
        ws->send(token);
    };
    ws->on_message = [recvLoginQueue](const std::string& msg) {
        recvLoginQueue->enqueue(msg);
    };
    ws->on_error = [](const std::string& err) {
        Logger::instance().error("Multiplayer game socket error: {}", err);
    };
    ws->on_close = []() {
        Logger::instance().error("Multiplayer game socket closed");
    };

    ws->connect();

    return ws;
}
