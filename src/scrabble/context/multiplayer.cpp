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
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "scrabble/sprites/tile.h"
#include "types_inspector.h"
#include "game_object/tween/tween.h"
#include "scrabble/actions/legal_actions.h"
#include "util/queue.h"
#include "util/network/sockets/web_socket.h"

using namespace scrabble;

namespace {
    WebSocketImpl *game_socket{nullptr};

    std::optional<MultiplayerGame> game_opt{std::nullopt};

    bool want_word_input_focus_{false};

    size_t user_index_;

    struct TileDrawData {
        float2 dimensions;
        float2 position;
        float rotation;
        Rectangle region;
    };

    std::vector<TileDrawData> public_tile_draw_data;
    std::vector<TileDrawData> word_tile_draw_data;

    std::optional<GameStateUpdate> last_action_;

    void DrawTiles() {
        const auto texture = Tile::GetTileTexture();
        for (const auto &data: public_tile_draw_data) {
            const auto xh = data.dimensions.x / 2.0f;
            const auto yh = data.dimensions.y / 2.0f;
            DrawTexturePro(texture.texture,
                           data.region,
                           {data.position.x + xh, data.position.y + yh, data.dimensions.x, data.dimensions.y},
                           {xh, yh},
                           data.rotation,
                           WHITE);
        }
        for (const auto &data: word_tile_draw_data) {
            const auto xh = data.dimensions.x / 2.0f;
            const auto yh = data.dimensions.y / 2.0f;
            DrawTexturePro(texture.texture,
                           data.region,
                           {data.position.x + xh, data.position.y + yh, data.dimensions.x, data.dimensions.y},
                           {xh, yh},
                           data.rotation,
                           WHITE);
        }
    }

    constexpr float DEFAULT_MARGIN = 8.0f;

    std::string start_action(const int player_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "START",
            .action = std::nullopt,
            .data = ""
        };
        return serialize(action);
    }

    std::string poll_action(const int player_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "POLL",
            .action = std::nullopt,
            .data = ""
        };
        return serialize(action);
    }

    std::string chat_action(const int player_id, const std::string &message) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "CHAT",
            .action = std::nullopt,
            .data = message
        };
        return serialize(action);
    }

    std::string buzz_action(const int player_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "BUZZ",
            .action = std::nullopt,
            .data = ""
        };
        return serialize(action);
    }

    std::string end_action(const int player_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "END",
            .action = std::nullopt,
            .data = ""
        };
        return serialize(action);
    }

    std::string flip_action(const int player_id, const int user_index, const std::string &tile_id) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "ACTION",
            .action = GameStateUpdate{
                .actionType = "FLIP",
                .flippedTileId = tile_id,
                .claimWord = "",
                .stolenWordId = "",
                .actingPlayer = user_index,
                .stolenPlayer = std::nullopt
            },
            .data = ""
        };
        return serialize(action);
    }

    std::string claim_action(const int player_id,
                             const int user_index,
                             const std::optional<int> stolen_user_index,
                             const std::string &stolen_word_id,
                             const std::string &claim_word) {
        const auto action = MultiplayerAction{
            .playerId = player_id,
            .actionType = "ACTION",
            .action = GameStateUpdate{
                .actionType = "CLAIM",
                .flippedTileId = "",
                .claimWord = claim_word,
                .stolenWordId = stolen_word_id,
                .actingPlayer = user_index,
                .stolenPlayer = stolen_user_index
            },
            .data = ""
        };
        return serialize(action);
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

    MarginContainer *margin_all(const float size) {
        using namespace frameflow;
        return new MarginContainer(MarginData{size, size, size, size});
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

    auto *p0 = margin_all(DEFAULT_MARGIN);
    layout_.left->AddChild(p0);
    p0->GetNode()->anchors = {0, 0, 1, 0};
    p0->GetNode()->expand.y = 1;

    auto *p1 = margin_all(DEFAULT_MARGIN);
    layout_.right->AddChild(p1);
    p1->GetNode()->anchors = {0, 0, 1, 0};
    p1->GetNode()->expand.y = 1;

    auto *p2 = margin_all(DEFAULT_MARGIN);
    layout_.left->AddChild(p2);
    p2->GetNode()->anchors = {0, 0, 1, 0};
    p2->GetNode()->expand.y = 1;

    auto *p3 = margin_all(DEFAULT_MARGIN);
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

MultiplayerContext::~MultiplayerContext() {
    if (game_socket) {
        game_socket->close();
        delete game_socket;
        game_socket = nullptr;
    }
}

void MultiplayerContext::Update(const float delta_time) {
    switch (state) {
        case State::PreInit:
            return;
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
            return;
        }
        default: break;
    }

    // State is Playing or Lobby
    if (state == State::Playing) {
        PollGameEvents();
    }
    time_since_last_poll += delta_time;
    if (time_since_last_poll > 0.100) {
        assert(game_socket != nullptr);
        game_socket->send(poll_action(main_menu->user_opt->id));
        time_since_last_poll = 0;
    }
    std::string msg;
    while (recv_game_queue.try_dequeue(msg)) {
        if (auto response = deserialize<MultiplayerActionResponse>(msg); response.ok) {
            if (!game_opt.has_value()) {
                auto it = std::find(
                    response.game->playerIds.begin(),
                    response.game->playerIds.end(),
                    main_menu->user_opt->id);
                user_index_ = std::distance(response.game->playerIds.begin(), it);
                game_opt = response.game;
                Logger::instance().info("Entering playing state with user index {}", user_index_);
                EnterPlaying();
                RedrawGame();
            }
            if (response.game->phase == "CREATED") {
                state = State::Lobby;
            } else if (response.game->phase == "ONGOING") {
                state = State::Playing;
            } else if (response.game->phase == "FINISHED") {
                state = State::Playing;
            }
            auto old_game = *game_opt;
            game_opt = response.game;
            if (response.game->lastAction != last_action_) {
                Logger::instance().info("Received a new action");
                last_action_ = response.game->lastAction;
                HandleAction(old_game, *response.game);
            }
        } else {
            Logger::instance().error("{}", response.errorMessage);
        }
    }
}

void MultiplayerContext::Draw() {
    switch (state) {
        case State::PreInit: {
            canvas->Hide();
            return;
        }
        case State::Gateway: {
            canvas->Hide();
            RenderGateway();
            return;
        }
        default: break;
    }
    if (should_redraw_layout) {
        RedrawLayout();
        RedrawGame();
        should_redraw_layout = false;
    }
    canvas->Show();
    RenderChat();
    ImGui::Begin("Multiplayer");
    if (state == State::Lobby) {
        RenderLobby();
    } else {
        RenderPlaying();
    }
    if (ImGui::Button("Back")) {
        ExitMultiplayer();
        main_menu->EnterMainMenu();
    }
    ImGui::End();
    DrawTiles();
}

/**
 * Should only be called when screen size changes for responsive layout redraw.
 */
void MultiplayerContext::RedrawLayout() {
    using namespace frameflow;

    layout_.middle->GetNode()->minimum_size = {(2 * DEFAULT_MARGIN + Tile::dim) * 10, 0};

    for (auto *child: layout_.public_tiles->GetChildren()) {
        child->Delete();
    }
    layout_.tile_slots.clear();
    for (int i = 0; i < 144; i++) {
        auto *outer = margin_all(DEFAULT_MARGIN);
        layout_.public_tiles->AddChild(outer);
        outer->GetNode()->minimum_size = {Tile::dim + DEFAULT_MARGIN * 2, Tile::dim + DEFAULT_MARGIN * 2};

        auto *inner = new Control();
        outer->AddChild(inner);
        inner->GetNode()->minimum_size = {Tile::dim, Tile::dim};
        inner->GetNode()->anchors = {0, 0, 1, 1};
        layout_.tile_slots.push_back(inner);
    }
}

// this should intake a thing
void MultiplayerContext::RedrawGame() const {
    if (state == State::Gateway || state == State::PreInit) return;
    if (!game_opt.has_value()) return;

    public_tile_draw_data.clear();
    word_tile_draw_data.clear();

    for (int i = 0; i < game_opt->state.tiles.size(); i++) {
        const auto &tile_props = game_opt->state.tiles[i];
        const auto *slot = layout_.tile_slots[i];
        const char letter = tile_props.faceUp ? tile_props.letter.front() : '\0';
        const auto region = Tile::GetTileTextureRegion(letter);
        public_tile_draw_data.push_back({
            {Tile::dim, Tile::dim},
            {slot->GetNode()->bounds.origin.x, slot->GetNode()->bounds.origin.y},
            0,
            Rectangle{region.x, region.y, 256, -256}
        });
    }

    for (int player_index = 0; player_index < game_opt->state.playerCount; player_index++) {
        auto *flow = layout_.player_flows[player_index];
        for (auto *child: flow->GetChildren()) {
            child->Delete();
        }
        for (const auto &[history, id]: game_opt->state.playerWords[player_index]) {
            const auto &word = history.front();
            auto *word_margin = margin_all(DEFAULT_MARGIN * 2.0f);
            flow->AddChild(word_margin);
            word_margin->GetNode()->minimum_size = {
                DEFAULT_MARGIN * 4 + (Tile::dim + 2 * 2) * static_cast<float>(word.length()),
                Tile::dim + 2 * 2 + DEFAULT_MARGIN * 4
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

                flow->ForceComputeLayout(); // todo, defer this after

                const auto region = Tile::GetTileTextureRegion(c);
                word_tile_draw_data.push_back({
                    {Tile::dim, Tile::dim},
                    {tile->GetNode()->bounds.origin.x, tile->GetNode()->bounds.origin.y},
                    0,
                    Rectangle{region.x, region.y, 256, -256}
                });
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
    ImGui::InputText("Game ID or URL", &foo);
    if (ImGui::Button("Join Game")) {
        // do new game socket thread
    }
    if (ImGui::Button("Back")) {
        ExitMultiplayer();
        main_menu->EnterMainMenu();
    }
    ImGui::End();
}

void MultiplayerContext::RenderChat() const {
    if (!game_opt) return;
    assert(game_socket != nullptr);

    ImGui::Begin("Chat", nullptr, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushFont(main_menu->monospace_font);

    // Calculate input height
    const float inputHeight = ImGui::GetTextLineHeight() * 3 + ImGui::GetStyle().FramePadding.y * 2;

    // Messages region
    {
        const float separatorHeight = ImGui::GetStyle().ItemSpacing.y;
        const float reservedHeight = inputHeight + separatorHeight;
        ImGui::BeginChild("ChatMessages",
                          ImVec2(0, -reservedHeight), // Reserve space for separator + input
                          ImGuiChildFlags_Borders);

        for (const auto &msg: game_opt->chat) {
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
        game_socket->send(chat_action(main_menu->user_opt->id, inputBuffer));
        inputBuffer.clear();
        setFocus = true; // Retain focus
    }

    ImGui::PopFont();
    ImGui::End();
}

void MultiplayerContext::RenderLobby() const {
    assert(game_socket != nullptr);
    if (ImGui::Button("Start Game")) {
        game_socket->send(start_action(main_menu->user_opt->id));
    }
}

void MultiplayerContext::RenderPlaying() const {
    assert(game_opt.has_value());
    assert(game_socket != nullptr);
    static std::string word_input;
    if (want_word_input_focus_) {
        ImGui::SetKeyboardFocusHere();
        want_word_input_focus_ = false;
    }
    constexpr ImGuiInputFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter;
    static auto filter = [](ImGuiInputTextCallbackData *data) {
        const ImWchar c = data->EventChar;
        if (c == ' ')
            return 1;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            if (c >= 'a' && c <= 'z')
                data->EventChar = c - 'a' + 'A'; // force uppercase
            return 0;
        }
        return 1;
    };
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 12)); // vertical + horizontal
    ImGui::PushFont(main_menu->big_font);

    ImGui::SetNextItemWidth(600.0f);

    if (ImGui::InputText("Word", &word_input, flags, filter)) {
        std::transform(word_input.begin(), word_input.end(), word_input.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        SendWord(word_input);
        word_input.clear();
    }

    ImGui::PopFont();
    ImGui::PopStyleVar();
    if (game_opt->phase == "ONGOING") {
        if (ImGui::Button("End Game")) {
            game_socket->send(end_action(main_menu->user_opt->id));
        }
    }
    ImGui::Text("%s", game_opt->phase.c_str());
    InspectStruct("game", *game_opt);
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
    Logger::instance().info("Entering game lobby: {}", game_id);
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
    Logger::instance().info("Entering playing");
    state = State::Playing;
}

void MultiplayerContext::ExitMultiplayer() {
    if (game_socket != nullptr) {
        game_socket->close();
        delete game_socket;
        game_socket = nullptr;
    }
    game_opt = std::nullopt;
    state = State::PreInit;
    last_action_ = std::nullopt;
}

void MultiplayerContext::PollGameEvents() const {
    const bool want_mouse = ImGui::GetIO().WantCaptureMouse;
    const bool left_mouse_down = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    const auto mouse_position = GetMousePosition();
    for (int i = 0; i < public_tile_draw_data.size(); i++) {
        const auto data = public_tile_draw_data[i];
        const Rectangle rect = {
            data.position.x,
            data.position.y,
            data.dimensions.x,
            data.dimensions.y
        };
        const bool is_over_rect = CheckCollisionPointRec(mouse_position, rect);
        if (!want_mouse && left_mouse_down && is_over_rect) {
            auto &tile_props = game_opt->state.tiles[i];
            FlipTile(tile_props.id);
        }
    }
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (IsKeyPressed(KEY_SPACE)) {
            want_word_input_focus_ = true;
        }
        if (IsKeyPressed(KEY_F)) {
            for (const auto &props: game_opt->state.tiles) {
                if (!props.faceUp) {
                    FlipTile(props.id);
                    break;
                }
            }
        }
    }
}

void MultiplayerContext::SendWord(const std::string &word) const {
    Logger::instance().info("Sending word: {}", word);

    game_socket->send(buzz_action(main_menu->user_opt->id));

    for (int i = 0; i < game_opt->state.playerCount; i++) {
        for (auto &steal_candidate: game_opt->state.playerWords[i]) {
            if (can_steal_word(word, steal_candidate, game_opt->state)) {
                game_socket->send(claim_action(main_menu->user_opt->id,
                                               static_cast<int>(user_index_),
                                               i,
                                               steal_candidate.id,
                                               word
                ));
                return;
            }
        }
    }

    // try to steal from public
    game_socket->send(claim_action(main_menu->user_opt->id,
                                   static_cast<int>(user_index_),
                                   std::nullopt,
                                   "",
                                   word));
}

void MultiplayerContext::FlipTile(const std::string &tile_id) const {
    Logger::instance().info("Sending flip: {}", tile_id);
    game_socket->send(flip_action(main_menu->user_opt->id,
                                  static_cast<int>(user_index_),
                                  tile_id));
}

void MultiplayerContext::HandleAction(const MultiplayerGame &old_state, const MultiplayerGame &new_state) {
    if (new_state.lastAction->actionType == "FLIP") {
        HandleFlipAction(old_state, new_state, *new_state.lastAction);
    } else if (new_state.lastAction->actionType == "CLAIM") {
        HandleClaimAction(old_state, new_state, *new_state.lastAction);
    }
}

void MultiplayerContext::HandleFlipAction(const MultiplayerGame &old_state, const MultiplayerGame &new_state,
                                          const GameStateUpdate &action) {
    Logger::instance().info("Received flip action");
    for (int i = 0; i < old_state.state.tiles.size(); i++) {
        if (action.flippedTileId == old_state.state.tiles[i].id) {
            const auto tween = TweenManager::instance().CreateTween(
                &public_tile_draw_data[i].rotation, 360, 0.25f, Easing::EaseInOutSine
            );
            tween->SetOnComplete([this] {
                this->RedrawGame();
            });
            break;
        }
    }
}

void MultiplayerContext::HandleClaimAction(const MultiplayerGame &old_state, const MultiplayerGame &new_state,
                                           const GameStateUpdate &action) {
    Logger::instance().info("Received claim action");

    int i = 0;
    for (auto &t: old_state.state.tiles) {
        if (get_tile_by_id(t.id, new_state.state) == std::nullopt) {
            TweenManager::instance().CreateTween(
                &public_tile_draw_data[i].position.x, 0, 4, Easing::EaseInOutSine
            );
            TweenManager::instance().CreateTween(
                &public_tile_draw_data[i].position.y, 0, 4, Easing::EaseInOutSine
            );
        }
        i++;
    }

    static float foo = 0;
    TweenManager::instance().CreateTween(
        &foo, 0, 4
    )->SetOnComplete([this] {
        this->RedrawGame();
    });

    // Peek a new layout (removing the old word from the flow and adding the new one
    // Animate tiles flowing to new layout
}
