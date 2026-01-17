#include "socket_client.h"

#include "types.h"
#include "util/logging/logging.h"

namespace scrabble {
    void UserLoginSocket(Queue &recvLoginQueue, const std::string &username, const std::string &password) {
        const std::string url = "wss://api.playpiratescrabble.com/ws/account/login";

        const auto payload = serialize(UserLoginAttempt{username, password});

        const WebSocket::Ptr ws = std::make_shared<WebSocket>(url);

        ws->on_open = [ws, payload] {
            ws->send(payload);
        };
        ws->on_message = [&recvLoginQueue](const std::string &msg) {
            recvLoginQueue.enqueue(msg);
        };
        ws->on_error = [](const std::string &err) {
            Logger::instance().error("Credential auth socket error: {}", err);
        };
        ws->on_close = []() {
        };

        ws->connect();

        // todo refactor to not use thread
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

    void TokenAuthSocket(Queue &recvLoginQueue, const std::string &token) {
        const std::string url = "wss://api.playpiratescrabble.com/ws/account/tokenAuth";
        const WebSocket::Ptr ws = std::make_shared<WebSocket>(url);

        ws->on_open = [ws, token] {
            ws->send(token);
        };
        ws->on_message = [&recvLoginQueue](const std::string &msg) {
            recvLoginQueue.enqueue(msg);
        };
        ws->on_error = [](const std::string &err) {
            Logger::instance().error("Token auth socket error: {}", err);
        };
        ws->on_close = []() {
        };

        ws->connect();

        // todo refactor to not use thread
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

    void NewGameSocket(Queue &recvLoginQueue, const std::string &token) {
        const std::string url = "wss://api.playpiratescrabble.com/ws/multiplayer/create";
        const WebSocket::Ptr ws = std::make_shared<WebSocket>(url);

        ws->on_open = [ws, token] {
            ws->send(token);
        };
        ws->on_message = [&recvLoginQueue](const std::string &msg) {
            recvLoginQueue.enqueue(msg);
        };
        ws->on_error = [](const std::string &err) {
            Logger::instance().error("New game socket error: {}", err);
        };
        ws->on_close = []() {
        };

        ws->connect();

        // todo refactor to not use thread
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

    WebSocketImpl *create_multiplayer_game_socket(Queue *recvLoginQueue, const std::string &token,
                                                  const std::string &game_id) {
        const std::string url = "wss://api.playpiratescrabble.com/ws/multiplayer/v2/" + game_id;
        auto *ws = new WebSocket(url);

        ws->on_open = [ws, token]() {
            Logger::instance().info("Multiplayer game socket connected");
            ws->send(token);
        };
        ws->on_message = [recvLoginQueue](const std::string &msg) {
            recvLoginQueue->enqueue(msg);
        };
        ws->on_error = [](const std::string &err) {
            Logger::instance().error("Multiplayer game socket error: {}", err);
        };
        ws->on_close = []() {
            Logger::instance().info("Multiplayer game socket closed");
        };

        ws->connect();

        return ws;
    }
}
