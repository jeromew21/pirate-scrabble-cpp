#include "sockets.h"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include "serialization/types.h"

void UserLoginSocket(Queue &recvLoginQueue, const std::string &username, const std::string &password) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/account/login";
    ix::WebSocket ws;
    ws.setUrl(url);
    ws.disableAutomaticReconnection();

    const auto payload = serialize(UserLoginAttempt{username, password});

    ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr &msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                recvLoginQueue.enqueue(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                ws.send(payload);
            } else if (msg->type == ix::WebSocketMessageType::Error) {
            }
        }
    );

    ws.start();

    // Wait for the server response (or timeout)
    std::this_thread::sleep_for(std::chrono::seconds(4));

    ws.stop(); // safely stops threads
    ws.close();
}

void TokenAuthSocket(Queue &recvLoginQueue, std::string token) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/account/tokenAuth";
    ix::WebSocket ws;
    ws.setUrl(url);
    ws.disableAutomaticReconnection();

    ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr &msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                recvLoginQueue.enqueue(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                ws.send(token);
            } else if (msg->type == ix::WebSocketMessageType::Error) {
            }
        }
    );

    ws.start();

    // Wait for the server response (or timeout)
    std::this_thread::sleep_for(std::chrono::seconds(4));

    ws.stop(); // safely stops threads
    ws.close();
}

void NewGameSocket(Queue &recvLoginQueue, std::string token) {
    const std::string url = "wss://api.playpiratescrabble.com/ws/multiplayer/create";
    ix::WebSocket ws;
    ws.setUrl(url);
    ws.disableAutomaticReconnection();

    ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr &msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                recvLoginQueue.enqueue(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                ws.send(token);
            } else if (msg->type == ix::WebSocketMessageType::Error) {
            }
        }
    );

    ws.start();

    // Wait for the server response (or timeout)
    std::this_thread::sleep_for(std::chrono::seconds(4));

    ws.stop(); // safely stops threads
    ws.close();
}
