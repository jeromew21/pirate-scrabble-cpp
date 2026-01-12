#pragma once

#include <string>

#include "util/queue.h"
#include "util/network/sockets/web_socket.h"

#ifdef __EMSCRIPTEN__
#include "util/network/sockets/web_socket_web.h"
using WebSocket = WebSocketWeb;
#else
#include "util/network/sockets/web_socket_desktop.h"
using WebSocket = WebSocketDesktop;
#endif

void UserLoginSocket(Queue &recvLoginQueue, const std::string &username, const std::string &password);

void TokenAuthSocket(Queue &recvLoginQueue, std::string token);

void NewGameSocket(Queue &recvLoginQueue, std::string token);

//void JoinGameSocket(Queue &recvLoginQueue, std::string token);

WebSocketImpl* create_multiplayer_game_socket(Queue *recvLoginQueue, const std::string& token, const std::string &game_id);

