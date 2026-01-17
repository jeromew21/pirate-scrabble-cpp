#pragma once

#include "types.h"
#include "game_object/game_object.h"

struct ImFont;

namespace scrabble {
    struct LoginContext;

    struct MultiplayerContext;

    struct MainMenuContext : GameObject {
        enum class State {
            InitialLoading,
            Menu,
            Multiplayer,
        };

        State state{State::InitialLoading};

        std::optional<User> user_opt{std::nullopt};

        LoginContext *login_context;

        MultiplayerContext *multiplayer_context;

        float loading_counter{0};

        float loading_time{2.0f};

        ImFont* big_font;

        ImFont* monospace_font;

        std::optional<std::function<void()>> request_exit_hook;

        explicit MainMenuContext(std::function<void()> request_exit);

        void Draw() override;

        void Update(float delta_time) override;

        void RenderMainMenu();

        void EnterMainMenu();
    };
}
