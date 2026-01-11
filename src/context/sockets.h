#pragma once

#include "../util/util.h"

#include <string>


void UserLoginSocket(Queue &recvLoginQueue, const std::string &username, const std::string &password);

void TokenAuthSocket(Queue &recvLoginQueue, std::string token);

void NewGameSocket(Queue &recvLoginQueue, std::string token);

//void JoinGameSocket(Queue &recvLoginQueue, std::string token);

