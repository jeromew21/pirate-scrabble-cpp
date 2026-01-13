#pragma once

#include "serialization/types.h"
#include "game_object/game_object.h"

struct LoginContext;

struct MultiplayerContext;

struct MainMenuContext : GameObject {
    enum class State {
        InitialLoading,
        Menu,
        Multiplayer,
    };

    State state{State::InitialLoading};

    PersistentData persistent_data{};

    std::optional<User> user_opt{std::nullopt};

    std::unique_ptr<LoginContext> login_context;

    std::unique_ptr<MultiplayerContext> multiplayer_context;

    std::string token_path{"./token.txt"};

    std::string persistent_data_path{"./pirate_scrabble_data.json"};

    float loading_counter{0};

    float loading_time{2.0f};

    MainMenuContext();

    ~MainMenuContext() override;

    void Draw() override;

    void Update(float delta_time) override;

    void RenderMainMenu();

    void EnterMainMenu();
};
