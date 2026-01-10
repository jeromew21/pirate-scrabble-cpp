#include <cfloat>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <thread>
#include <unordered_map>

#include <raylib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>

#include <imgui.h>
#include <rlImGui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <frameflow/layout.hpp>

#include "util.h"
#include "text/texthb.h"
#include "serialization/types.h"
#include "sockets.h"
#include "ixwebsocket/IXWebSocket.h"


using namespace frameflow;

static Color color_for(NodeType type) {
    switch (type) {
        case NodeType::Center: return BLUE;
        case NodeType::Box: return GREEN;
        case NodeType::Flow: return ORANGE;
        case NodeType::Generic: return RAYWHITE;
        default: return MAGENTA;
    }
}

static const char *node_type_name(NodeType type) {
    switch (type) {
        case NodeType::Center: return "Center";
        case NodeType::Box: return "Box";
        case NodeType::Flow: return "Flow";
        case NodeType::Generic: return "Generic";
        default: return "Unknown";
    }
}

static void DrawNodeRects(System &sys, NodeId id) {
    const Node &node = get_node(sys, id);

    Rectangle r{
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y
    };

    if (get_node(sys, id).parent != NullNode) {
        Color c = color_for(node.type);
        DrawRectangleLinesEx(r, 1.0f, c);

        // Draw node type text in top-left corner
        const char *type_str = node_type_name(node.type);
        int font_size = 12;
        DrawText(type_str,
                 static_cast<int>(node.bounds.origin.x) + 2,
                 static_cast<int>(node.bounds.origin.y) + 2,
                 font_size,
                 c);
    }

    for (NodeId child: node.children) {
        DrawNodeRects(sys, child);
    }
}

static Rectangle rl_rect(Rect r) {
    return {r.origin.x, r.origin.y, r.size.x, r.size.y};
}

struct MainMenuContext {
    struct LoginContext {
        enum class State {
            PreLogin,
            Bypassed,
            Active,
            Loading,
        };

        MainMenuContext *parent;
        Queue recv_login_queue;
        State state = State::PreLogin;
        std::string logs;
        std::string username_label;
        std::string password_label;

        void attempt_token_auth(std::string token) {
            std::cout << "Attempting token authentication" << std::endl;
            std::thread t(TokenAuthSocket, std::ref(recv_login_queue),  token);
            t.detach();
        }

        void handle_incoming() {
            std::string msg;
            while (recv_login_queue.try_dequeue(msg)) {
                auto response = deserialize<UserResponse>(msg);
                if (response.ok) {
                    logs.append("USER AUTH SUCCESS\n");
                    std::cout << "USER AUTH SUCCESS\n";
                    // some callback, maybe
                    parent->authenticate_user(response.user.value());
                    state = State::Bypassed;
                } else {
                    logs.append("USER AUTH FAIL: " + response.error + "\n");
                    std::cout << "USER AUTH FAIL\n";
                    state = State::Active;
                }
            }
        }

        void render() {
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
    };

    struct MultiplayerContext {
        enum class State {
            PreInit, Gateway, Lobby, Playing
        };

        std::optional<MultiplayerGame> game;

        MainMenuContext *parent;

        State state = State::PreInit;

        ix::WebSocket game_socket;

        Queue recv_create_queue;

        Queue recv_join_queue;

        Queue recv_game_queue;

        void handle_incoming() {
            switch (state) {
                case State::PreInit:
                    break;
                case State::Gateway: {
                    // join or create
                    std::string msg;
                    while (recv_create_queue.try_dequeue(msg)) {
                        auto response = deserialize<MultiplayerActionResponse>(msg);
                        if (response.ok) {
                            std::cout << "NEW GAME CREATED!" << std::endl;
                            game = response.game;
                            // start the socket?
                            enter_lobby();
                        } else {
                            std::cerr << response.errorMessage << std::endl;
                        }
                    }
                    while (recv_join_queue.try_dequeue(msg)) {
                        std::cout << msg;
                    }
                    break;
                }
                case State::Playing:
                case State::Lobby: {
                    std::string msg;
                    while (recv_game_queue.try_dequeue(msg)) {
                        break;
                    }
                    break;
                }
            }
        }

        void render_gateway() {
            ImGui::Begin("Multiplayer");
            if (ImGui::Button("New Game")) {
                // do new game socket thread
                std::thread t(NewGameSocket, std::ref(recv_create_queue), parent->user->token);
                t.detach();
            }
            if (ImGui::Button("Join Game")) {
                // do new game socket thread
            }
            if (ImGui::Button("Back")) {
                parent->enter_main_menu();
            }
            ImGui::End();
        }

        void render_lobby() {
            ImGui::Begin("Game Lobby");
            ImGui::End();
        }

        void render_playing() {
            ImGui::Begin("Gameplay Debug");
            ImGui::End();
        }

        void enter_gateway() {
            state = State::Gateway;
            parent->login_context.attempt_token_auth(parent->user->token);
            // re authenticate in parallel?
            // re authenticate every X seconds?
            if (parent->user->currentGame.has_value()) {
                enter_playing();
            }
        }

        void enter_lobby() {
            state = State::Lobby;
        }

        void enter_playing() {
            state = State::Playing;
        }

        void render() {
            if (state == State::Gateway) {
                handle_incoming();
                render_gateway();
            } else if (state == State::Lobby) {
                handle_incoming();
                render_lobby();
            } else if (state == State::Playing) {
                handle_incoming();
                render_playing();
            } else {
                // preinit
            }
        }
    };

    enum class State {
        InitialLoading,
        Menu,
        Multiplayer,
    };

    State state = State::InitialLoading;

    PersistentData persistent_data;

    std::optional<User> user;

    LoginContext login_context;

    MultiplayerContext multiplayer_context;

    std::string token_path = "./token.txt";

    std::string persistent_data_path = "./gamedata.json";

    float loading_counter = 0;

    float loading_time = 2.0f;

    void authenticate_user(User new_user) {
        user = new_user;
    }

    bool load_persistent_data() {
        std::string outContents;
        read_file(persistent_data_path, outContents);
        try {
            persistent_data = deserialize_or_throw<PersistentData>(outContents);
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return false;
        }
        return true;
    }

    bool write_persistent_data() const {
        return write_file(persistent_data_path, serialize(persistent_data));
    }

    bool write_token() const {
        if (user.has_value()) {
            return write_file(token_path, user->token);
        }
        return true;
    }

    void render_main_menu() {
        ImGui::Begin("Main Menu");

        if (ImGui::Button("Multiplayer")) {
            // set state to multiplayer gateway
            state = State::Multiplayer;
            multiplayer_context.enter_gateway();
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

    void enter_main_menu() {
        state = State::Menu;
        if (user.has_value()) {
            login_context.attempt_token_auth(user->token);
        }
    }

    void render(const float delta) {
        login_context.handle_incoming();
        switch (state) {
            case State::InitialLoading:
            {
                loading_counter += delta;
                if (loading_counter > loading_time) {
                    state = State::Menu;
                    if (!user.has_value()) {
                        login_context.state = LoginContext::State::Active;
                    }
                }
                break;
            }
            case State::Multiplayer: {
                multiplayer_context.render();
                break;
            }
            case State::Menu: {
                if (login_context.state == LoginContext::State::Bypassed) {
                    render_main_menu();
                } else {
                    login_context.render();
                }
            }
        }
    }

    MainMenuContext() {
        std::cout << "Initializing main context." << std::endl;
        load_persistent_data();
        login_context.parent = this;
        multiplayer_context.parent = this;
        if (std::string token; read_file(token_path, token)) {
            std::erase_if(token, ::isspace);
            login_context.attempt_token_auth(token);
        }
    }

    ~MainMenuContext() {
        if (!write_persistent_data()) {
            std::cout << "Failed to write persistent data to disk." << std::endl;
        }
        if (!write_token()) {
            std::cout << "Failed to write token to disk." << std::endl;
        }
    }
};


int main() {
    // Make window resizable
    SetTraceLogLevel(LOG_NONE); //
    InitWindow(1920/2, 1080/2, "Pirate Scrabble");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    rlImGuiSetup(true); // sets up ImGui with either a dark or light default theme

    // set up contexts
    MainMenuContext menu_context{};

    // -------------------------
    // Initialize FreeType
    // -------------------------
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Failed to init FreeType\n";
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "arial.ttf", 0, &face)) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    HBFont font(face, 48); // pixel size 48

    System ui;
    NodeId root = add_generic(ui, NullNode);

    NodeId loading_node = add_center(ui, root);
    NodeId loading_text_node = add_generic(ui, loading_node);
    {
        get_node(ui, loading_node).anchors = {0, 0, 1, 1};
        get_node(ui, loading_text_node).anchors = {.5, 0.5, 0.5, 0.5};
        get_node(ui, loading_text_node).minimum_size = {400, 100};
    }

    NodeId multiplayer_node = add_box(ui, root, BoxData{Direction::Horizontal, Align::Start});
    get_node(ui, multiplayer_node).anchors = {0, 0, 1, 1};

    {
        NodeId left = add_generic(ui, multiplayer_node);
        get_node(ui, left).anchors = {0, 0, 0, 1};
        get_node(ui, left).minimum_size = {100, 0};
        get_node(ui, left).expand.x = 1;
    }

    std::vector<NodeId> tileIds;

    {
        NodeId middle = add_generic(ui, multiplayer_node);
        get_node(ui, middle).anchors = {0, 0, 0, 1};
        get_node(ui, middle).minimum_size = {48 * 10, 0};
        NodeId vbox = add_box(ui, middle, BoxData{Direction::Vertical, Align::Start});
        NodeId title = add_generic(ui, vbox);
        get_node(ui, title).minimum_size = {0, 200};
        get_node(ui, vbox).anchors = {0, 0, 1, 1};
        NodeId flow = add_flow(ui, vbox, FlowData{Direction::Horizontal, Align::Start});
        get_node(ui, flow).anchors = {0, 0, 1, 1};
        for (int i = 0; i < 144; i++) {
            NodeId tile = add_generic(ui, flow);
            get_node(ui, tile).minimum_size = {48, 48};
            tileIds.push_back(tile);
        }
    }

    {
        NodeId right = add_generic(ui, multiplayer_node);
        get_node(ui, right).anchors = {0, 0, 0, 1};
        get_node(ui, right).minimum_size = {100, 0};
        get_node(ui, right).expand.x = 1;
    }

    while (!WindowShouldClose()) {
        {
            // Poll window size
            int win_width = GetScreenWidth();
            int win_height = GetScreenHeight();
            Node &root_node = get_node(ui, root);
            root_node.bounds.size = {float(win_width), float(win_height)};
            root_node.minimum_size = {float(win_width), float(win_height)};
        }

        // Compute layout
        compute_layout(ui, root);

        // Draw
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);

            rlImGuiBegin();

            menu_context.render(GetFrameTime());

            //DrawNodeRects(ui, loading_node);


            if (menu_context.state == MainMenuContext::State::InitialLoading) {
                Node node = get_node(ui, loading_text_node);
                std::string text = "Loading...";
                auto extents = MeasureTextHB(font, text);
                float2 offset{
                    (node.bounds.size.x - extents.width) * 0.5f,
                    (node.bounds.size.y - extents.height) * 0.5f
                };
                DrawTextHB(font, text, node.bounds.origin.x, node.bounds.origin.y + extents.ascent, BLACK);
            }

            /*
            DrawTextHB(font, "Hello, world!", 50, 300, BLACK);
            DrawTextHB(font, "f i ligatures!", 50, 360, RED);
            for (auto tile_id: tileIds) {
                auto node = get_node(ui, tile_id);
                Rectangle rec = rl_rect(node.bounds);
                DrawRectangleRoundedLinesEx(rec, 0.1, 5, 1, RED);
                auto extents = MeasureTextHB(font, "A");
                float2 offset{
                    (node.bounds.size.x - extents.width) * 0.5f,
                    (node.bounds.size.y - extents.height) * 0.5f
                };
                float2 text_pos = offset + node.bounds.origin;
                DrawTextHB(font, "A", text_pos.x, text_pos.y + extents.ascent, BLACK);
            }
            */

            rlImGuiEnd();
        }
        EndDrawing();
    }
    // todo: shutdown harfbuzz/freetype
    std::cout << "Shutting down." << std::endl;

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
