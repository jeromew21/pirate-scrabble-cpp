#include "main_menu.h"

#include <iostream>

#include "raylib.h"

#include "frameflow/layout.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "login.h"
#include "multiplayer.h"
#include "text/texthb.h"
#include "util/filesystem.h"
#include "util/logging/logging.h"
#include "serialization/types.h"

static bool write_persistent_data(const std::string &persistent_data_path, const PersistentData &data) {
    return write_file(persistent_data_path, serialize(data));
}

static bool write_token(const std::string &token_path, const std::string &token) {
    return write_file(token_path, token);
}

static std::optional<PersistentData> load_persistent_data(const std::string &persistent_data_path) {
    if (std::string contents; read_file(persistent_data_path, contents)) {
        try {
            return deserialize_or_throw<PersistentData>(contents);
        } catch (const std::exception &e) {
            std::cerr << e.what() << "\n";
            return std::nullopt;
        }
    }
    return std::nullopt;
}

MainMenuContext::MainMenuContext() : login_context(std::make_unique<LoginContext>()),
                                     multiplayer_context(std::make_unique<MultiplayerContext>()) {
    Logger::instance().info("Initializing main context");
    if (const auto persistent_data_opt = load_persistent_data(persistent_data_path);
        persistent_data_opt.has_value()) {
        persistent_data = persistent_data_opt.value();
    } else {
        Logger::instance().info("Failed to read persistent data from disk. Falling back to defaults");
    }

    AddChild(login_context.get());
    AddChild(multiplayer_context.get());
    login_context->main_menu = this;
    multiplayer_context->main_menu = this;
    if (std::string token; read_file(token_path, token)) {
        std::erase_if(token, ::isspace);
        login_context->AttemptTokenAuth(token);
    } else {
        login_context->state = LoginContext::State::Active;
        Logger::instance().info("Failed to read token from disk. User must provide credentials");
    }
}

MainMenuContext::~MainMenuContext() {
    Logger::instance().info("Shutting down main context");
    if (!write_persistent_data(persistent_data_path, persistent_data)) {
        Logger::instance().info("Failed to write persistent data to disk");
    }
    if (user_opt.has_value()) {
        if (!write_token(token_path, user_opt->token)) {
            Logger::instance().info("Failed to write token to disk");
        }
    }
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
                if (!user.has_value()) {
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

    ImGui::End();
}

void MainMenuContext::EnterMainMenu() {
    state = State::Menu;
    if (user_opt.has_value()) {
        login_context->AttemptTokenAuth(user_opt->token);
    }
}
