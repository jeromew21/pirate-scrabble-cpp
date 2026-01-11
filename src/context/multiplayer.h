#pragma once

#include <iostream>
#include <ixwebsocket/IXWebSocket.h>

#include "../serialization/types.h"
#include "../util/util.h"
#include "game_object/game_object.h"

struct MainMenuContext;

struct MultiplayerContext : GameObject {
        enum class State {
            PreInit, Gateway, Lobby, Playing
        };

        std::optional<MultiplayerGame> game;

        MainMenuContext *main_menu;

        State state = State::PreInit;

        ix::WebSocket game_socket;

        Queue recv_create_queue;

        Queue recv_join_queue;

        Queue recv_game_queue;

    /*

        void handle_incoming() {
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

        void render_gateway() {
            ImGui::Begin("Multiplayer");
            if (ImGui::Button("New Game")) {
                // do new game socket thread
                std::thread t(NewGameSocket, std::ref(recv_create_queue), parent->user->token);
                t.detach();
            }
            if (ImGui::Button("Join Game")) {
                // do new game socket thread
            }
            if (ImGui::Button("Back")) {
                parent->enter_main_menu();
            }
            ImGui::End();
        }

        void render_lobby() {
            ImGui::Begin("Game Lobby");
            ImGui::End();
        }

        void render_playing() {
            ImGui::Begin("Gameplay Debug");
            ImGui::End();
        }

        void enter_gateway() {
            state = State::Gateway;
            parent->login_context.attempt_token_auth(parent->user->token);
            // re authenticate in parallel?
            // re authenticate every X seconds?
            if (parent->user->currentGame.has_value()) {
                enter_playing();
            }
        }

        void enter_lobby() {
            state = State::Lobby;
        }

        void enter_playing() {
            state = State::Playing;
        }

        void render() {
            if (state == State::Gateway) {
                handle_incoming();
                render_gateway();
            } else if (state == State::Lobby) {
                handle_incoming();
                render_lobby();
            } else if (state == State::Playing) {
                handle_incoming();
                render_playing();
            } else {
                // preinit
            }
        }
        */
};