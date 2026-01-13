#pragma once

#include "serialization/types.h"
#include "util/queue.h"
#include "util/network/sockets/web_socket.h"
#include "game_object/game_object.h"

struct MainMenuContext;

struct BoxContainer;

struct MultiplayerContext : GameObject {
    enum class State {
        PreInit, Gateway, Lobby, Playing
    };

private:
    State state = State::PreInit;

public:
    std::optional<MultiplayerGame> game_opt;

    BoxContainer *node;

    MainMenuContext *main_menu;

    WebSocketImpl *game_socket{nullptr};

    Queue recv_create_queue; // Can we combine both of these? Idk why not...
    //Queue recv_join_queue;

    Queue recv_game_queue; // Active game

    float time_since_last_poll{0};

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
