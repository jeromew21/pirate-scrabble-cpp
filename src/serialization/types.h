#pragma once

#include <string>
#include <optional>

#include "json.hpp"

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

// Add this template specialization for std::optional support
namespace nlohmann {
    template<typename T>
    struct adl_serializer<std::optional<T> > {
        static void to_json(json &j, const std::optional<T> &opt) {
            if (opt) {
                j = *opt;
            } else {
                j = nullptr;
            }
        }

        static void from_json(const json &j, std::optional<T> &opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };
}

// Serialization / Deserialization helpers
template<typename T>
std::string serialize(const T &obj) {
    using namespace nlohmann;
    json j = obj;
    return j.dump();
}

template<typename T>
T deserialize(const std::string &jsonString) {
    using namespace nlohmann;
    json j = json::parse(jsonString);
    return j.get<T>();
}

// Templated function that throws on error
template<typename T>
T deserialize_or_throw(const std::string &str) {
    using namespace nlohmann;
    try {
        json j = json::parse(str);
        return j.get<T>();
    } catch (const json::parse_error &e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    } catch (const json::type_error &e) {
        throw std::runtime_error(std::string("JSON type error: ") + e.what());
    }
}
