#pragma once

#include "game_object/game_object.h"
#include "util/queue.h"

struct BoxContainer;

struct Control;

struct LayoutSystem;

struct FlowContainer;

struct MarginContainer;

namespace scrabble {
    struct MainMenuContext;

    struct MultiplayerGame;

    struct GameStateUpdate;

    struct MultiplayerContext : GameObject {
        enum class State {
            PreInit, Gateway, Lobby, Playing
        };

        struct Layout {
            BoxContainer *left;
            BoxContainer *middle;
            BoxContainer *right;
            FlowContainer *public_tiles;
            std::vector<Control *> tile_slots;
            std::vector<FlowContainer *> player_flows;
        };

    private:
        State state = State::PreInit;

        Layout layout_{};

    public:
        MainMenuContext *main_menu{nullptr};

        Queue recv_create_queue; // Can we combine both of these? Not sure why not...
        //Queue recv_join_queue;

        Queue recv_game_queue; // Active game

        float time_since_last_poll{0};

        bool should_redraw_layout{false};

        LayoutSystem *canvas;

        MultiplayerContext();

        ~MultiplayerContext() override;

        void Update(float delta_time) override;

        void Draw() override;

        void RedrawLayout();

        void RedrawGame() const;

        void RenderGateway();

        void RenderChat() const;

        void RenderLobby() const;

        void RenderPlaying() const;

        void EnterGateway();

        void EnterLobby(const std::string &game_id);

        void EnterPlaying();

        void PollGameEvents() const;

        void SendWord(const std::string &word) const;

        void FlipTile(const std::string &tile_id) const;

        void HandleAction(const MultiplayerGame &old_state, const MultiplayerGame &new_state);

        void HandleFlipAction(const MultiplayerGame &old_state, const MultiplayerGame &new_state,
                              const GameStateUpdate &action);

        void HandleClaimAction(const MultiplayerGame &old_state, const MultiplayerGame &new_state,
                               const GameStateUpdate &action);

        // exit playing?
    };
}
