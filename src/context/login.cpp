#include "login.h"

#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <fmt/base.h>

#include "main_menu.h"
#include "sockets.h"
#include "serialization/types.h"
#include "util/math.h"

void LoginContext::Update(float delta_time) {
    std::string msg;
    while (recv_login_queue.try_dequeue(msg)) {
        auto response = deserialize<UserResponse>(msg);
        if (main_menu->state == MainMenuContext::State::InitialLoading) {
            main_menu->state = MainMenuContext::State::Menu;
        }
        if (response.ok) {
            // some callback, maybe
            main_menu->AuthenticateUser(response.user.value());
            state = State::Bypassed;
        } else {
            state = State::Active;
        }
    }
}

void LoginContext::Draw() {
    if (state == State::Active || state == State::Loading) {
        ImGui::Begin("Log in");
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        bool b1 = ImGui::InputText("Username", &username_label, flags);
        bool b2 = ImGui::InputText("Password", &password_label, flags);
        bool b3 = ImGui::Button("Log in");
        if (b1 || b2 || b3) {
            // todo: loading state
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
