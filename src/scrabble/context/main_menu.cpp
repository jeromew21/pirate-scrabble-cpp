#include "main_menu.h"

#include <utility>

#include "frameflow/layout.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "login.h"
#include "multiplayer.h"
#include "util/filesystem/filesystem.h"
#include "util/logging/logging.h"
#include "types.h"

using namespace scrabble;

MainMenuContext::MainMenuContext(std::function<void()> request_exit)
    : login_context(new LoginContext()),
      multiplayer_context(new MultiplayerContext()),
      request_exit_hook(std::move(request_exit)) {
    Logger::instance().info("Initializing main context");
    AddChild(login_context);
    AddChild(multiplayer_context);
    login_context->main_menu = this;
    multiplayer_context->main_menu = this;
}

void MainMenuContext::Draw() {
    using namespace frameflow;

    /*
    if (state == State::InitialLoading) {
        Node node = get_node(ui, loading_text_node);
        std::string text = "Loading...";
        auto extents = MeasureTextHB(font, text);
        float2 offset{
            (node.bounds.size.x - extents.width) * 0.5f,
            (node.bounds.size.y - extents.height) * 0.5f
        };
        DrawTextHB(font, text, node.bounds.origin.x, node.bounds.origin.y + extents.ascent, BLACK);
    }
    */

    switch (state) {
        case State::InitialLoading:
            break;
        case State::Multiplayer: {
            break;
        }
        case State::Menu: {
            if (login_context->state == LoginContext::State::Bypassed) {
                RenderMainMenu();
            }
        }
    }
}

void MainMenuContext::Update(const float delta_time) {
    switch (state) {
        case State::InitialLoading: {
            /*
            loading_counter += delta_time;
            if (loading_counter > loading_time) {
                state = State::Menu;
                if (!user) {
                    login_context->state = LoginContext::State::Active;
                }
            }
            */
            break;
        }
        case State::Multiplayer:
        case State::Menu:
            break;
    }
}

void MainMenuContext::RenderMainMenu() {
    ImGui::Begin("Main Menu");

    if (ImGui::Button("Multiplayer")) {
        // set state to multiplayer gateway
        state = State::Multiplayer;
        multiplayer_context->EnterGateway();
    }

    if (ImGui::Button("Exit")) {
        if (request_exit_hook) {
            (*request_exit_hook)();
        }
    }

    if (ImGui::CollapsingHeader("Settings")) {
        ImGui::Text("Adjust graphics, audio, and controls.");
        // Example settings sliders
        static float volume = 1.0f;
        ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Profile")) {
        ImGui::Text("User profile and statistics.");
        ImGui::Text("Username: %s", user_opt->username.c_str());
    }

    if (ImGui::CollapsingHeader("About")) {
        ImGui::Text("Pirate Scrabble v1.0");
        ImGui::Text("Copyright 2026");
    }

    ImGui::Checkbox("Show debug window", show_debug_window);

    ImGui::End();
}

void MainMenuContext::EnterMainMenu() {
    state = State::Menu;
    if (user_opt) {
        login_context->AttemptTokenAuth(user_opt->token);
    }
}
