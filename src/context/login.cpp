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
#include "util/util.h"

void LoginContext::Update(float delta_time) {
    std::string msg;
    while (recv_login_queue.try_dequeue(msg)) {
        auto response = deserialize<UserResponse>(msg);
        if (response.ok) {
            logs.append("USER AUTH SUCCESS\n");
            std::cout << "USER AUTH SUCCESS\n";
            // some callback, maybe
            main_menu->authenticate_user(response.user.value());
            state = State::Bypassed;
        } else {
            logs.append("USER AUTH FAIL: " + response.error + "\n");
            std::cout << "USER AUTH FAIL\n";
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
        if (ImGui::CollapsingHeader("Socket Response Logs (incomplete)")) {
            ImGui::Text(logs.c_str());
        }
        ImGui::End();
    }
}

void LoginContext::attempt_token_auth(std::string token) {
    fmt::println("Attempting token authentication");
    std::thread t(TokenAuthSocket, std::ref(recv_login_queue),  token);
    t.detach();
}
