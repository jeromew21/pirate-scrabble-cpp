#include "multiplayer.h"

#include <iostream>
#include <cassert>

#include "imgui.h"
#include "imgui_stdlib.h"

#include "types.h"
#include "login.h"
#include "util/logging/logging.h"
#include "main_menu.h"
#include "socket_client.h"
#include "external/cgltf.h"
#include "game_object/entity/sprite.h"
#include "game_object/tween/tween.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "scrabble/sprites/tile.h"
#include "types_inspector.h"
#include "util/queue.h"
#include "util/network/sockets/web_socket.h"

namespace {
    WebSocketImpl *game_socket{nullptr};

    std::optional<MultiplayerGame> game{std::nullopt};


    struct TileDrawData {
        float2 dimensions;
        float2 position;
        Rectangle region;
    };

    std::vector<TileDrawData> tile_draw_data;

    void DrawTiles() {
        const auto texture = Tile::GetTileTexture();
        for (const auto &data: tile_draw_data) {
            DrawTexturePro(texture.texture,
                           data.region,
                           {data.position.x, data.position.y, data.dimensions.x, data.dimensions.y},
                           {0, 0},
                           0,
                           WHITE);
        }
    }

    constexpr float margin = 8.0f;

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

    layout_.left = new BoxContainer(BoxData{Direction::Vertical, Align::Start});
    layout_.right = new BoxContainer(BoxData{Direction::Vertical, Align::Start});

    auto *hbox = horizontal_box();
    canvas->AddChild(hbox);
    hbox->GetNode()->anchors = {0, 0, 1, 1};

    hbox->AddChild(layout_.left);
    layout_.left->GetNode()->anchors = {0, 0, 0, 1};
    layout_.left->GetNode()->minimum_size = {100, 0};
    layout_.left->GetNode()->expand.x = 1;

    layout_.middle = dynamic_cast<BoxContainer *>(hbox->AddChild(vertical_box()));
    hbox->AddChild(layout_.middle);
    layout_.middle->GetNode()->anchors = {0, 0, 1, 1};

    layout_.public_tiles = dynamic_cast<FlowContainer *>(layout_.middle->AddChild(horizontal_flow()));
    layout_.public_tiles->GetNode()->anchors = {0, 0, 1, 1};
    layout_.public_tiles->GetNode()->expand.y = 1;

    hbox->AddChild(layout_.right);
    layout_.right->GetNode()->anchors = {0, 0, 1, 1};
    layout_.right->GetNode()->minimum_size = {100, 0};
    layout_.right->GetNode()->expand.x = 1;

    auto *p0 = margin_all(margin);
    layout_.left->AddChild(p0);
    p0->GetNode()->anchors = {0, 0, 1, 0};
    p0->GetNode()->expand.y = 1;

    auto *p1 = margin_all(margin);
    layout_.right->AddChild(p1);
    p1->GetNode()->anchors = {0, 0, 1, 0};
    p1->GetNode()->expand.y = 1;

    auto *p2 = margin_all(margin);
    layout_.left->AddChild(p2);
    p2->GetNode()->anchors = {0, 0, 1, 0};
    p2->GetNode()->expand.y = 1;

    auto *p3 = margin_all(margin);
    layout_.right->AddChild(p3);
    p3->GetNode()->anchors = {0, 0, 1, 0};
    p3->GetNode()->expand.y = 1;

    std::vector<Control *> players = {p0, p1, p2, p3};
    for (auto *player: players) {
        auto *vbox = vertical_box();
        player->AddChild(vbox);
        vbox->GetNode()->anchors = {0, 0, 1, 1};


        auto *flow = horizontal_flow();
        vbox->AddChild(flow);
        flow->GetNode()->anchors = {0, 0, 1, 1};
        flow->GetNode()->expand.y = 1;
        layout_.player_flows.push_back(flow);
    }

    RedrawLayout();
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
                    RedrawGame(); //force redraw after every update
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
            if (should_redraw_layout) {
                RedrawLayout();
                should_redraw_layout = false;
            }
            canvas->Show();
            ImGui::Begin("Multiplayer Debug");
            RenderChat();
            if (state == State::Lobby) {
                RenderLobby();
            } else {
                RenderPlaying();
            }
            ImGui::End();
            DrawTiles();
            break;
        }
    }
}

void MultiplayerContext::RedrawLayout() {
    using namespace frameflow;

    layout_.middle->GetNode()->minimum_size = {(2 * margin + Tile::dim) * 10, 0};

    for (auto *child: layout_.public_tiles->GetChildren()) {
        child->Delete();
    }
    layout_.tile_slots.clear();
    for (int i = 0; i < 144; i++) {
        auto *outer = margin_all(margin);
        layout_.public_tiles->AddChild(outer);
        outer->GetNode()->minimum_size = {Tile::dim + margin * 2, Tile::dim + margin * 2};

        auto *inner = new Control();
        outer->AddChild(inner);
        inner->GetNode()->minimum_size = {Tile::dim, Tile::dim};
        inner->GetNode()->anchors = {0, 0, 1, 1};
        layout_.tile_slots.push_back(inner);
    }
}

void MultiplayerContext::RedrawGame() {
    if (state == State::Gateway || state == State::PreInit) return;
    if (!game.has_value()) return;

    tile_draw_data.clear();

    {
        int i = 0;
        for (auto tile_props: game->state.tiles) {
            const auto *slot = layout_.tile_slots[i];

            auto region = Tile::GetTileTextureRegion(tile_props.letter.front());
            tile_draw_data.push_back({
                {Tile::dim, Tile::dim},
                {slot->GetNode()->bounds.origin.x, slot->GetNode()->bounds.origin.y},
                Rectangle{region.x, region.y, 256, -256}
            });

            i++;
        }
    }


    for (int player_index = 0; player_index < game->state.playerCount; player_index++) {
        auto *flow = layout_.player_flows[player_index];
        for (auto *child: flow->GetChildren()) {
            child->Delete();
        }
        for (const auto &[history, id]: game->state.playerWords[player_index]) {
            const auto &word = history.front();
            auto *word_margin = margin_all(margin * 2.0f);
            flow->AddChild(word_margin);
            word_margin->GetNode()->minimum_size = {
                margin * 4 + (Tile::dim + 2 * 2) * (float) word.length(), Tile::dim + 2 * 2 + margin * 4
            };
            auto *word_box = horizontal_box();
            word_margin->AddChild(word_box);
            word_box->GetNode()->anchors = {0, 0, 1, 1};
            for (const auto c: word) {
                auto *small_margin = margin_all(2);
                word_box->AddChild(small_margin);
                small_margin->GetNode()->minimum_size = {Tile::dim + 2 * 2, Tile::dim + 2 * 2};
                const auto tile = new Control();
                small_margin->AddChild(tile);
                tile->GetNode()->minimum_size = {Tile::dim, Tile::dim};
                tile->GetNode()->anchors = {0, 0, 1, 1};

                flow->ForceComputeLayout();

                auto region = Tile::GetTileTextureRegion(c);
                tile_draw_data.push_back({
                    {Tile::dim, Tile::dim},
                    {tile->GetNode()->bounds.origin.x, tile->GetNode()->bounds.origin.y},
                    Rectangle{region.x, region.y, 256, -256}
                });

                /*
                auto *sprite = new Sprite();
                sprite->SetTexture(Tile::GetTileTexture().texture, ::float2{Tile::dim, Tile::dim});
                auto region = Tile::GetTileTextureRegion(c);
                sprite->SetTextureRegion({region.x, region.y, 256, -256});
                sprite->transform.local_position = {tile->GetNode()->bounds.origin.x, tile->GetNode()->bounds.origin.y};
                tile->AddChild(sprite);
                */
            }
        }
    }
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
}

void MultiplayerContext::EnterPlaying() {
    state = State::Playing;
}
