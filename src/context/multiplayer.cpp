#include "multiplayer.h"

#include <iostream>
#include <cassert>

#include "imgui.h"
#include "imgui_stdlib.h"

#include "login.h"
#include "util/logging/logging.h"
#include "main_menu.h"
#include "sockets.h"
#include "serialization/types_inspector.h"

static MultiplayerAction start_action(const int player_id) {
    const auto action = MultiplayerAction{
        .playerId = player_id,
        .actionType = "START",
        .action = std::nullopt,
        .data = ""
    };
    return action;
}

static MultiplayerAction poll_action(const int player_id) {
    const auto action = MultiplayerAction{
        .playerId = player_id,
        .actionType = "POLL",
        .action = std::nullopt,
        .data = ""
    };
    return action;
}

void MultiplayerContext::Update(float delta_time) {
    switch (state) {
        case State::PreInit:
            break;
        case State::Gateway: {
            // join or create
            std::string msg;
            while (recv_create_queue.try_dequeue(msg)) {
                Logger::instance().info("{}", msg);
                if (auto response = deserialize<MultiplayerActionResponse>(msg); response.ok) {
                    Logger::instance().info("New game created");
                    EnterLobby(response.game->id);
                } else {
                    Logger::instance().error("{}", response.errorMessage);
                }
            }
            /*
            while (recv_join_queue.try_dequeue(msg)) {
            }
            */
            break;
        }
        case State::Playing:
        case State::Lobby: {
            time_since_last_poll += delta_time;
            if (time_since_last_poll > 0.100) {
                assert(game_socket != nullptr);
                game_socket->send(serialize(poll_action(main_menu->user->id)));
                time_since_last_poll = 0;
            }
            std::string msg;
            while (recv_game_queue.try_dequeue(msg)) {
                if (auto response = deserialize<MultiplayerActionResponse>(msg); response.ok) {
                    if (game_opt.has_value()) {
                        if (state == State::Lobby && response.game->phase != "CREATED") {
                            EnterPlaying();
                        }
                    }
                    if (response.game->phase == "CREATED") {
                        state = State::Lobby;
                    } else {
                        state = State::Playing;
                    }
                    game_opt = response.game;
                } else {
                    Logger::instance().error("{}", response.errorMessage);
                }
                break;
            }
            break;
        }
    }
}

void MultiplayerContext::Draw() {
    switch (state) {
        case State::PreInit:
            break;
        case State::Gateway:
            RenderGateway();
            break;
        case State::Playing:
        case State::Lobby: {
            ImGui::Begin("Multiplayer Debug");
            if (state == State::Lobby) {
                RenderLobby();
            } else {
                RenderPlaying();
            }
            ImGui::End();
            break;
        }
    }
}

void MultiplayerContext::RenderGateway() {
    ImGui::Begin("Multiplayer Gateway");
    if (ImGui::Button("New Game")) {
        // do new game socket thread
        std::thread t(NewGameSocket, std::ref(recv_create_queue), main_menu->user->token);
        t.detach();
    }
    static std::string foo;
    ImGui::InputText("", &foo);
    if (ImGui::Button("Join Game")) {
        // do new game socket thread
    }
    if (ImGui::Button("Back")) {
        main_menu->EnterMainMenu();
    }
    ImGui::End();
}

void MultiplayerContext::RenderLobby() const {
    assert(game_socket != nullptr);
    if (ImGui::Button("Start Game")) {
        game_socket->send(serialize(start_action(main_menu->user->id)));
    }
}

void MultiplayerContext::RenderPlaying() const {
    assert(game_opt.has_value());
    const MultiplayerGame game = *game_opt;
    ImGui::Text("%s", game.phase.c_str());
    InspectStruct("game", game);
}

void MultiplayerContext::EnterGateway() {
    state = State::Gateway;
    main_menu->login_context->AttemptTokenAuth(main_menu->user->token);
    // re authenticate in parallel?
    // re authenticate every X seconds?
    if (main_menu->user->currentGame.has_value()) {
        EnterLobby(main_menu->user->currentGame.value());
    }
}

void MultiplayerContext::EnterLobby(const std::string &game_id) {
    state = State::Lobby;
    if (game_socket != nullptr) {
        Logger::instance().info("Closing and resetting socket");
        game_socket->close();
        delete game_socket;
        game_socket = nullptr;
    }
    game_socket = create_multiplayer_game_socket(&recv_game_queue,
                                                 main_menu->user->token,
                                                 game_id);
    time_since_last_poll = 0;
}

void MultiplayerContext::EnterPlaying() {
    state = State::Playing;
}
