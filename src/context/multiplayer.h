#pragma once

#include <iostream>

#include "serialization/types.h"
#include "util/util.h"
#include "game_object/game_object.h"

struct MainMenuContext;

struct MultiplayerContext : GameObject {
    enum class State {
        PreInit, Gateway, Lobby, Playing
    };

private:
    State state = State::PreInit;

public:
    std::optional<MultiplayerGame> game;

    MainMenuContext *main_menu;

    //ix::WebSocket game_socket;

    Queue recv_create_queue;

    Queue recv_join_queue;

    Queue recv_game_queue;

    void Update(float delta_time) override;

    void Draw() override;

    void render_gateway();

    void render_lobby();

    void render_playing();

    void enter_gateway();

    void enter_lobby();

    void enter_playing();

    // exit playing?
};
