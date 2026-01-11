#include "multiplayer.h"

#include "imgui.h"
#include <imgui_stdlib.h>

#include "login.h"
#include "main_menu.h"
#include "sockets.h"

void MultiplayerContext::Update(float delta_time) {
    switch (state) {
        case State::PreInit:
            break;
        case State::Gateway: {
            // join or create
            std::string msg;
            while (recv_create_queue.try_dequeue(msg)) {
                auto response = deserialize<MultiplayerActionResponse>(msg);
                if (response.ok) {
                    std::cout << "NEW GAME CREATED!" << std::endl;
                    game = response.game;
                    // start the socket?
                    enter_lobby();
                } else {
                    std::cerr << response.errorMessage << std::endl;
                }
            }
            while (recv_join_queue.try_dequeue(msg)) {
                std::cout << msg;
            }
            break;
        }
        case State::Playing:
        case State::Lobby: {
            std::string msg;
            while (recv_game_queue.try_dequeue(msg)) {
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
            render_gateway();
            break;
        case State::Playing:
            render_playing();
            break;
        case State::Lobby:
            render_lobby();
            break;
    }
}

void MultiplayerContext::render_gateway() {
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
        main_menu->enter_main_menu();
    }
    ImGui::End();
}

void MultiplayerContext::render_lobby() {
    ImGui::Begin("Game Lobby");
    ImGui::End();
}

void MultiplayerContext::render_playing() {
    ImGui::Begin("Gameplay Debug");
    ImGui::End();
}

void MultiplayerContext::enter_gateway() {
    state = State::Gateway;
    main_menu->login_context->attempt_token_auth(main_menu->user->token);
    // re authenticate in parallel?
    // re authenticate every X seconds?
    if (main_menu->user->currentGame.has_value()) {
        enter_playing();
    }
}

void MultiplayerContext::enter_lobby() {
    state = State::Lobby;
}

void MultiplayerContext::enter_playing() {
    state = State::Playing;
}
