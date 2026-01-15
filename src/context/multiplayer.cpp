#include "multiplayer.h"

#include <iostream>
#include <cassert>

#include "imgui.h"
#include "imgui_stdlib.h"

#include "login.h"
#include "util/logging/logging.h"
#include "main_menu.h"
#include "socket_client.h"
#include "external/cgltf.h"
#include "game_object/entity/sprite.h"
#include "game_object/tween/tween.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "scrabble/tile.h"
#include "serialization/types_inspector.h"

namespace {
    MultiplayerAction start_action(const int player_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "START",
            .action = std::nullopt,
            .data = ""
        };
        return action;
    }

    MultiplayerAction poll_action(const int player_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "POLL",
            .action = std::nullopt,
            .data = ""
        };
        return action;
    }

    MultiplayerAction chat_action(const int player_id, const std::string &message) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "CHAT",
            .action = std::nullopt,
            .data = message
        };
        return action;
    }

    FlowContainer *horizontal_flow() {
        using namespace frameflow;
        return new FlowContainer(FlowData{
            Direction::Horizontal, Align::Start
        });
    }

    BoxContainer *horizontal_box() {
        using namespace frameflow;
        return new BoxContainer(BoxData{
            Direction::Horizontal, Align::Start
        });
    }

    BoxContainer *vertical_box() {
        using namespace frameflow;
        return new BoxContainer(BoxData{
            Direction::Vertical, Align::Start
        });
    }

    MarginContainer *margin_all(const float margin) {
        using namespace frameflow;
        return new MarginContainer(MarginData{margin, margin, margin, margin});
    }
}

MultiplayerContext::MultiplayerContext() {
    using namespace frameflow;
    canvas = new LayoutSystem();
    canvas->Hide();
    AddChild(canvas);
}

void MultiplayerContext::Update(const float delta_time) {
    switch (state) {
        case State::PreInit:
            break;
        case State::Gateway: {
            // join or create
            std::string msg;
            while (recv_create_queue.try_dequeue(msg)) {
                Logger::instance().info("{}", msg);
                if (auto response = deserialize<MultiplayerActionResponse>(msg); response.ok) {
                    Logger::instance().info("New game created");
                    EnterLobby(response.game->id);
                } else {
                    Logger::instance().error("{}", response.errorMessage);
                }
            }
            /*
            while (recv_join_queue.try_dequeue(msg)) {
            }
            */
            break;
        }
        case State::Playing:
        case State::Lobby: {
            time_since_last_poll += delta_time;
            if (time_since_last_poll > 0.100) {
                assert(game_socket != nullptr);
                game_socket->send(serialize(poll_action(main_menu->user_opt->id)));
                time_since_last_poll = 0;
            }
            std::string msg;
            while (recv_game_queue.try_dequeue(msg)) {
                if (auto response = deserialize<MultiplayerActionResponse>(msg); response.ok) {
                    if (game) {
                        if (state == State::Lobby && response.game->phase != "CREATED") {
                            EnterPlaying();
                        }
                    }
                    if (response.game->phase == "CREATED") {
                        state = State::Lobby;
                    } else {
                        state = State::Playing;
                    }
                    game = response.game;
                    Redraw();
                } else {
                    Logger::instance().error("{}", response.errorMessage);
                }
                break;
            }
            break;
        }
    }
}

void MultiplayerContext::Draw() {
    switch (state) {
        case State::PreInit: {
            canvas->Hide();
            break;
        }
        case State::Gateway: {
            canvas->Hide();
            RenderGateway();
            break;
        }
        case State::Playing:
        case State::Lobby: {
            canvas->Show();
            ImGui::Begin("Multiplayer Debug");
            RenderChat();
            if (state == State::Lobby) {
                RenderLobby();
            } else {
                RenderPlaying();
            }
            ImGui::End();
            break;
        }
    }
}

void MultiplayerContext::Redraw() {
    using namespace frameflow;
    {
        auto children = canvas->GetChildren();
        for (auto *child: children) {
            child->Delete();
        }
    }

    auto *hbox = horizontal_box();
    canvas->AddChild(hbox);
    hbox->GetNode()->anchors = {0, 0, 1, 1};

    auto *left = new BoxContainer(BoxData{Direction::Vertical, Align::Start});
    hbox->AddChild(left);
    left->GetNode()->anchors = {0, 0, 0, 1};
    left->GetNode()->minimum_size = {100, 0};
    left->GetNode()->expand.x = 1;

    auto *center_container = margin_all(8);
    hbox->AddChild(center_container);
    center_container->GetNode()->anchors = {0, 0, 1, 1};
    center_container->GetNode()->minimum_size = {Tile::dim * 10, 0};

    auto *center = dynamic_cast<BoxContainer *>(center_container->AddChild(vertical_box()));
    center->GetNode()->anchors = {0, 0, 1, 1};

    auto *public_tiles = dynamic_cast<FlowContainer *>(center_container->AddChild(horizontal_flow()));
    public_tiles->GetNode()->anchors = {0, 0, 1, 1};
    public_tiles->GetNode()->expand.y = 1;
    for (int i = 0; i < 144; i++) {
        auto *outer = new MarginContainer(MarginData{8, 8, 8, 8});
        public_tiles->AddChild(outer);
        outer->GetNode()->minimum_size = {Tile::dim + 8 * 2, Tile::dim + 8 * 2};

        auto *inner = new Control();
        outer->AddChild(inner);
        inner->GetNode()->minimum_size = {Tile::dim, Tile::dim};
        inner->GetNode()->anchors = {0, 0, 1, 1};
    }

    auto *right = new BoxContainer(BoxData{Direction::Vertical, Align::Start});
    hbox->AddChild(right);
    right->GetNode()->anchors = {0, 0, 1, 1};
    right->GetNode()->minimum_size = {100, 0};
    right->GetNode()->expand.x = 1;

}


void MultiplayerContext::RenderGateway() {
    ImGui::Begin("Multiplayer Gateway");
    if (ImGui::Button("New Game")) {
        // do new game socket thread
        std::thread t(NewGameSocket, std::ref(recv_create_queue), main_menu->user_opt->token);
        t.detach();
    }
    static std::string foo;
    ImGui::InputText("", &foo);
    if (ImGui::Button("Join Game")) {
        // do new game socket thread
    }
    if (ImGui::Button("Back")) {
        main_menu->EnterMainMenu();
    }
    ImGui::End();
}

void MultiplayerContext::RenderChat() const {
    if (!game) return;
    assert(game_socket != nullptr);

    ImGui::Begin("Chat", nullptr, ImGuiWindowFlags_NoScrollbar);

    // Calculate input height
    const float inputHeight = ImGui::GetTextLineHeight() * 3 + ImGui::GetStyle().FramePadding.y * 2;

    // Messages region
    {
        const float separatorHeight = ImGui::GetStyle().ItemSpacing.y;
        const float reservedHeight = inputHeight + separatorHeight;
        ImGui::BeginChild("ChatMessages",
                          ImVec2(0, -reservedHeight), // Reserve space for separator + input
                          ImGuiChildFlags_Borders);

        for (const auto &msg: game->chat) {
            // Timestamp
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[%s]", msg.timestamp.c_str());
            ImGui::SameLine();

            // Sender
            if (msg.sender) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s:", msg.sender->c_str());
                ImGui::SameLine();
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "[system]:");
                ImGui::SameLine();
            }

            // Message text (wrapped) - use 0 for full width
            ImGui::PushTextWrapPos(0.0f); // 0 = wrap at edge of window
            ImGui::TextUnformatted(msg.message.c_str());
            ImGui::PopTextWrapPos();
        }

        // Auto-scroll to bottom when new messages arrive
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }

    // Input box
    ImGui::Separator();

    static std::string inputBuffer;
    static bool setFocus = false;

    // Input field - CtrlEnterForNewLine returns true when Enter is pressed
    bool enterPressed = ImGui::InputTextMultiline(
        "##ChatInput",
        &inputBuffer,
        ImVec2(-1.0f, inputHeight),
        ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue
    );

    if (setFocus) {
        ImGui::SetKeyboardFocusHere(-1);
        setFocus = false;
    }

    // Send message when Enter is pressed (without Ctrl)
    if (enterPressed && !inputBuffer.empty()) {
        game_socket->send(serialize(chat_action(main_menu->user_opt->id, inputBuffer)));
        inputBuffer.clear();
        setFocus = true; // Retain focus
    }

    ImGui::End();
}

void MultiplayerContext::RenderLobby() const {
    assert(game_socket != nullptr);
    if (ImGui::Button("Start Game")) {
        game_socket->send(serialize(start_action(main_menu->user_opt->id)));
    }
}

void MultiplayerContext::RenderPlaying() const {
    assert(game.has_value());
    ImGui::Text("%s", game->phase.c_str());
    InspectStruct("game", *game);
}

void MultiplayerContext::EnterGateway() {
    state = State::Gateway;
    main_menu->login_context->AttemptTokenAuth(main_menu->user_opt->token);
    // re authenticate in parallel?
    // re authenticate every X seconds?
    if (main_menu->user_opt->currentGame) {
        EnterLobby(main_menu->user_opt->currentGame.value());
    }
}

void MultiplayerContext::EnterLobby(const std::string &game_id) {
    state = State::Lobby;
    if (game_socket != nullptr) {
        Logger::instance().info("Closing and resetting socket");
        game_socket->close();
        delete game_socket;
        game_socket = nullptr;
    }
    game_socket = create_multiplayer_game_socket(&recv_game_queue,
                                                 main_menu->user_opt->token,
                                                 game_id);
    time_since_last_poll = 0;

    auto *sprite = new Sprite();
    sprite->SetTexture(Tile::GetTileTexture('X').texture);
    sprite->transform.local_position = {100, 100};
    AddChild(sprite);
    auto *tween = TweenManager::instance().CreateTween(&sprite->transform.local_position.x,
                                                       200,
                                                       1.0f,
                                                       Easing::EaseInOutSine);
    tween->SetOnComplete([=] { sprite->Delete(); });
}

void MultiplayerContext::EnterPlaying() {
    state = State::Playing;
}
