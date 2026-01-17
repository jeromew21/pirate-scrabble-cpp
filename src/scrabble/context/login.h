#pragma once

#include <string>

#include "game_object/game_object.h"
#include "util/queue.h"

namespace scrabble {
    struct MainMenuContext;

    struct LoginContext : GameObject {
        enum class State {
            PreLogin,
            Bypassed,
            Active,
            Loading,
        };

        MainMenuContext *main_menu;

        Queue recv_login_queue;

        State state = State::PreLogin;

        std::string username_label;

        std::string password_label;

        void Update(float delta_time) override;

        void Draw() override;

        void AttemptTokenAuth(const std::string &token);
    };
}
