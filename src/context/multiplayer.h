#pragma once

#include <iostream>

#include "serialization/types.h"
#include "util/util.h"
#include "game_object/game_object.h"
#include "network/sockets/web_socket.h"

struct MainMenuContext;

struct MultiplayerContext : GameObject {
    enum class State {
        PreInit, Gateway, Lobby, Playing
    };

private:
    State state = State::PreInit;

public:
    std::optional<MultiplayerGame> game_opt;

    MainMenuContext *main_menu;

    WebSocketImpl *game_socket{nullptr};

    Queue recv_create_queue; // Can we combine both of these? Idk why not...
    //Queue recv_join_queue;

    Queue recv_game_queue; // Active game

    void Update(float delta_time) override;

    void Draw() override;

    void RenderGateway();

    void RenderLobby() const;

    void RenderPlaying() const;

    void EnterGateway();

    void EnterLobby(const std::string &game_id);

    void EnterPlaying();

    // exit playing?
};
