#include "login.h"

#include <functional>
#include <string>
#include <thread>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <fmt/base.h>

#include "main_menu.h"
#include "sockets.h"
#include "serialization/types.h"
#include "util/logging/logging.h"

void LoginContext::Update(float delta_time) {
    std::string msg;
    while (recv_login_queue.try_dequeue(msg)) {
        auto response = deserialize<UserResponse>(msg);
        if (main_menu->state == MainMenuContext::State::InitialLoading) {
            main_menu->state = MainMenuContext::State::Menu;
        }
        if (response.ok) {
            Logger::instance().info("Authenticated as {}", response.user->username);
            main_menu->user_opt = response.user.value();
            state = State::Bypassed;
        } else {
            state = State::Active;
        }
    }
}

void LoginContext::Draw() {
    if (state == State::Active) {
        ImGui::Begin("Log in");
        constexpr ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        const bool b1 = ImGui::InputText("Username", &username_label, flags);
        const bool b2 = ImGui::InputText("Password", &password_label, flags | ImGuiInputTextFlags_Password);
        const bool b3 = ImGui::Button("Log in");
        if (b1 || b2 || b3) {
            // todo: loading state
            // do we even need a thread, we can use main thread to release the socket...
            std::thread t(UserLoginSocket, std::ref(recv_login_queue), username_label, password_label);
            t.detach();
        }
        ImGui::End();
    }
}

void LoginContext::AttemptTokenAuth(const std::string& token) {
    std::thread t(TokenAuthSocket, std::ref(recv_login_queue),  token);
    t.detach();
}
