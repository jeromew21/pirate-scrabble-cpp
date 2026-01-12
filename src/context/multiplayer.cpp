#include "multiplayer.h"

#include <iostream>
#include <cassert>

#include "imgui.h"
#include "imgui_stdlib.h"

#include "fmt/core.h"

#include "login.h"
#include "main_menu.h"
#include "sockets.h"

static MultiplayerAction start_action(const int player_id) {
    const auto action = MultiplayerAction{
        .playerId = player_id,
        .actionType = "START",
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
                fmt::println("{}", msg);
                if (auto response = deserialize<MultiplayerActionResponse>(msg); response.ok) {
                    std::cout << "NEW GAME CREATED!" << std::endl;
                    EnterLobby(response.game->id);
                } else {
                    std::cerr << response.errorMessage << std::endl;
                }
            }
            /*
            while (recv_join_queue.try_dequeue(msg)) {
                std::cout << msg;
            }
            */
            break;
        }
        case State::Playing:
        case State::Lobby: {
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
                    std::cerr << response.errorMessage << std::endl;
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
            RenderPlaying();
            break;
        case State::Lobby:
            RenderLobby();
            break;
    }
}

void MultiplayerContext::RenderGateway() {
    ImGui::Begin("Multiplayer");
    if (ImGui::Button("New Game")) {
        // do new game socket thread
        std::thread t(NewGameSocket, std::ref(recv_create_queue), main_menu->user->token);
        t.detach();
    }
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
    ImGui::Begin("Game Lobby");
    if (ImGui::Button("Start Game")) {
        game_socket->send(serialize(start_action(main_menu->user->id)));
    }
    ImGui::End();
}

void MultiplayerContext::RenderPlaying() const {
    assert(game_opt.has_value());
    const MultiplayerGame game = *game_opt;
    ImGui::Begin("Gameplay Debug");
    ImGui::Text("%s", game.phase.c_str());
    ImGui::End();
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
    // check game socket, reconnect
    game_socket = create_multiplayer_game_socket(&recv_game_queue,
                                                 main_menu->user->token,
                                                 game_id);
}

void MultiplayerContext::EnterPlaying() {
    state = State::Playing;
}
