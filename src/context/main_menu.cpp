#include "main_menu.h"

#include <fmt/core.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "login.h"
#include "multiplayer.h"
#include "../util/util.h"
#include "../serialization/types.h"

static bool write_persistent_data(const std::string &persistent_data_path, const PersistentData &data) {
    return write_file(persistent_data_path, serialize(data));
}

static bool write_token(const std::string &token_path, const std::string &token) {
    return write_file(token_path, token);
}

static std::optional<PersistentData> load_persistent_data(const std::string &persistent_data_path) {
    std::string outContents;
    read_file(persistent_data_path, outContents);
    try {
        return deserialize_or_throw<PersistentData>(outContents);
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return std::nullopt;
    }
    return std::nullopt;
}

MainMenuContext::MainMenuContext() : login_context(std::make_unique<LoginContext>()),
                                     multiplayer_context(std::make_unique<MultiplayerContext>()) {
    fmt::println("Initializing main context");
    if (const auto persistent_data_opt = load_persistent_data(persistent_data_path);
        persistent_data_opt.has_value()) {
        persistent_data = persistent_data_opt.value();
    }
    AddChild(login_context.get());
    AddChild(multiplayer_context.get());
    login_context->main_menu = this;
    multiplayer_context->main_menu = this;
    if (std::string token; read_file(token_path, token)) {
        std::erase_if(token, ::isspace);
        fmt::println("Read token from disk.");
        login_context->attempt_token_auth(token);
    }
}

MainMenuContext::~MainMenuContext() {
    if (!write_persistent_data(persistent_data_path, persistent_data)) {
        fmt::println("Failed to write persistent data to disk.");
    }
    if (user.has_value()) {
        if (!write_token(token_path, user->token)) {
            fmt::println("Failed to write token to disk.");
        }
    }
}

void MainMenuContext::authenticate_user(User new_user) {
    user = new_user;
}

void MainMenuContext::Draw() {
    switch (state) {
        case State::InitialLoading:
            break;
        case State::Multiplayer: {
            break;
        }
        case State::Menu: {
            if (login_context->state == LoginContext::State::Bypassed) {
                render_main_menu();
            }
        }
    }
}

void MainMenuContext::Update(const float delta_time) {
    switch (state) {
        case State::InitialLoading: {
            loading_counter += delta_time;
            if (loading_counter > loading_time) {
                state = State::Menu;
                if (!user.has_value()) {
                    login_context->state = LoginContext::State::Active;
                }
            }
            break;
        }
        case State::Multiplayer:
            break;
        case State::Menu:
            break;
    }
}

void MainMenuContext::render_main_menu() {
    ImGui::Begin("Main Menu");

    if (ImGui::Button("Multiplayer")) {
        // set state to multiplayer gateway
        state = State::Multiplayer;
        //multiplayer_context.enter_gateway();
    }

    if (ImGui::CollapsingHeader("Settings")) {
        ImGui::Text("Adjust graphics, audio, and controls.");
        // Example settings sliders
        static float volume = 1.0f;
        ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Profile")) {
        ImGui::Text("User profile and statistics.");
        ImGui::Text("Username: %s", user->username.c_str());
    }

    if (ImGui::CollapsingHeader("About")) {
        ImGui::Text("Pirate Scrabble v1.0");
        ImGui::Text("Copyright 2026");
    }

    ImGui::End();
}
