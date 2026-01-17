#pragma once

#include <string>
#include <optional>

#include "util/serialization/serialization.h"

namespace scrabble {
    struct PersistentData {
        int window_width{1920};
        int window_height{1080};
        bool show_debug_window{true};
        int dummy{0};
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        PersistentData,
        window_width,
        window_height,
        show_debug_window,
        dummy
    )

    struct TileProps {
        std::string letter;
        std::string id;
        bool faceUp;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileProps, letter, id, faceUp)

    struct Word {
        std::vector<std::string> history;
        std::string id;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Word, history, id)

    struct GameState {
        int tileCount;
        int wordMinimumSize;
        int playerCount;
        std::vector<TileProps> tiles;
        std::string dictionary;
        std::vector<std::vector<Word> > playerWords;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        GameState,
        tileCount,
        wordMinimumSize,
        playerCount,
        tiles,
        dictionary,
        playerWords
    )

    struct GameStateUpdate {
        std::string actionType;
        std::string flippedTileId;
        std::string claimWord;
        std::string stolenWordId;
        int actingPlayer;
        std::optional<int> stolenPlayer;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        GameStateUpdate,
        actionType,
        flippedTileId,
        claimWord,
        stolenWordId,
        actingPlayer,
        stolenPlayer
    )

    struct MultiplayerChatMessage {
        std::optional<std::string> sender;
        std::string timestamp;
        std::string message;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        MultiplayerChatMessage,
        sender,
        timestamp,
        message
    )

    struct MultiplayerAction {
        int playerId;
        std::string actionType;
        std::optional<GameStateUpdate> action;
        std::string data;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        MultiplayerAction,
        playerId,
        actionType,
        action,
        data
    )

    struct MultiplayerGame {
        std::string id;
        std::string phase;
        GameState state;
        std::vector<int> playerIds;
        std::vector<std::string> playerNames;
        std::optional<int> buzzHolder;
        std::optional<int> buzzElapsed;
        std::optional<GameStateUpdate> lastAction;
        std::vector<MultiplayerChatMessage> chat;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        MultiplayerGame,
        id,
        phase,
        state,
        playerIds,
        playerNames,
        buzzHolder,
        buzzElapsed,
        lastAction,
        chat
    )

    struct MultiplayerActionResponse {
        bool ok;
        std::optional<MultiplayerGame> game;
        int hashCode;
        std::string errorMessage;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        MultiplayerActionResponse,
        ok,
        game,
        hashCode,
        errorMessage
    )

    struct User {
        int id;
        std::string username;
        std::string token;
        std::string salt; // You probably shouldn't have this - security risk!
        std::string email;
        std::optional<std::string> currentGame;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, id, username, token, salt, email, currentGame)

    struct UserLoginAttempt {
        std::string username;
        std::string password;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserLoginAttempt, username, password)

    struct UserResponse {
        bool ok;
        std::string error;
        std::optional<User> user;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserResponse, ok, error, user)
}
