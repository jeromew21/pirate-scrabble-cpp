#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <optional>

// Helper functions for displaying different types
namespace ImGuiInspect {
    inline void Field(const char* label, const std::string& value) {
        ImGui::Text("%s: %s", label, value.c_str());
    }

    inline void Field(const char* label, int value) {
        ImGui::Text("%s: %d", label, value);
    }

    inline void Field(const char* label, bool value) {
        ImGui::Text("%s: %s", label, value ? "true" : "false");
    }

    template<typename T>
    inline void Field(const char* label, const std::optional<T>& opt) {
        if (opt.has_value()) {
            Field(label, *opt);
        } else {
            ImGui::TextDisabled("%s: None", label);
        }
    }

    template<typename T>
    inline void VectorField(const char* label, const std::vector<T>& vec);
}

// Macro to start inspecting a struct
#define IMGUI_INSPECT_BEGIN(structName, obj) \
    if (ImGui::TreeNode(structName)) {

#define IMGUI_INSPECT_END() \
        ImGui::TreePop(); \
    }

// Macro to inspect a simple field
#define IMGUI_INSPECT_FIELD(obj, field) \
    ImGuiInspect::Field(#field, obj.field)

// Forward declarations for struct inspectors
void InspectStruct(const char* label, const PersistentData& data);
void InspectStruct(const char* label, const TileProps& tile);
void InspectStruct(const char* label, const Word& word);
void InspectStruct(const char* label, const GameState& state);
void InspectStruct(const char* label, const GameStateUpdate& update);
void InspectStruct(const char* label, const MultiplayerChatMessage& msg);
void InspectStruct(const char* label, const MultiplayerAction& action);
void InspectStruct(const char* label, const MultiplayerGame& game);
void InspectStruct(const char* label, const MultiplayerActionResponse& response);
void InspectStruct(const char* label, const User& user);
void InspectStruct(const char* label, const UserLoginAttempt& attempt);
void InspectStruct(const char* label, const UserResponse& response);

// Template specialization for vectors of structs
namespace ImGuiInspect {
    template<typename T>
    inline void VectorField(const char* label, const std::vector<T>& vec) {
        if (ImGui::TreeNode(label, "%s [%zu]", label, vec.size())) {
            for (size_t i = 0; i < vec.size(); i++) {
                std::string itemLabel = std::to_string(i);
                if constexpr (std::is_same_v<T, std::string>) {
                    ImGui::Text("[%zu]: %s", i, vec[i].c_str());
                } else if constexpr (std::is_arithmetic_v<T>) {
                    ImGui::Text("[%zu]: %d", i, static_cast<int>(vec[i]));
                } else {
                    InspectStruct(itemLabel.c_str(), vec[i]);
                }
            }
            ImGui::TreePop();
        }
    }

    // Specialization for vector of vectors
    template<typename T>
    inline void VectorField(const char* label, const std::vector<std::vector<T>>& vec) {
        if (ImGui::TreeNode(label, "%s [%zu]", label, vec.size())) {
            for (size_t i = 0; i < vec.size(); i++) {
                std::string itemLabel = std::string(label) + "[" + std::to_string(i) + "]";
                VectorField(itemLabel.c_str(), vec[i]);
            }
            ImGui::TreePop();
        }
    }
}

// Implementation of struct inspectors
inline void InspectStruct(const char* label, const PersistentData& data) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(data, dummy);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const TileProps& tile) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(tile, letter);
        IMGUI_INSPECT_FIELD(tile, id);
        IMGUI_INSPECT_FIELD(tile, faceUp);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const Word& word) {
    if (ImGui::TreeNode(label)) {
        ImGuiInspect::VectorField("history", word.history);
        IMGUI_INSPECT_FIELD(word, id);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const GameState& state) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(state, tileCount);
        IMGUI_INSPECT_FIELD(state, wordMinimumSize);
        IMGUI_INSPECT_FIELD(state, playerCount);
        ImGuiInspect::VectorField("tiles", state.tiles);
        IMGUI_INSPECT_FIELD(state, dictionary);
        ImGuiInspect::VectorField("playerWords", state.playerWords);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const GameStateUpdate& update) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(update, actionType);
        IMGUI_INSPECT_FIELD(update, flippedTileId);
        IMGUI_INSPECT_FIELD(update, claimWord);
        IMGUI_INSPECT_FIELD(update, stolenWordId);
        IMGUI_INSPECT_FIELD(update, actingPlayer);
        IMGUI_INSPECT_FIELD(update, stolenPlayer);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const MultiplayerChatMessage& msg) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(msg, sender);
        IMGUI_INSPECT_FIELD(msg, timestamp);
        IMGUI_INSPECT_FIELD(msg, message);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const MultiplayerAction& action) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(action, playerId);
        IMGUI_INSPECT_FIELD(action, actionType);
        if (action.action.has_value()) {
            InspectStruct("action", *action.action);
        } else {
            ImGui::TextDisabled("action: None");
        }
        IMGUI_INSPECT_FIELD(action, data);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const MultiplayerGame& game) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(game, id);
        IMGUI_INSPECT_FIELD(game, phase);
        InspectStruct("state", game.state);
        ImGuiInspect::VectorField("playerIds", game.playerIds);
        ImGuiInspect::VectorField("playerNames", game.playerNames);
        IMGUI_INSPECT_FIELD(game, buzzHolder);
        IMGUI_INSPECT_FIELD(game, buzzElapsed);
        if (game.lastAction.has_value()) {
            InspectStruct("lastAction", *game.lastAction);
        } else {
            ImGui::TextDisabled("lastAction: None");
        }
        ImGuiInspect::VectorField("chat", game.chat);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const MultiplayerActionResponse& response) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(response, ok);
        if (response.game.has_value()) {
            InspectStruct("game", *response.game);
        } else {
            ImGui::TextDisabled("game: None");
        }
        IMGUI_INSPECT_FIELD(response, hashCode);
        IMGUI_INSPECT_FIELD(response, errorMessage);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const User& user) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(user, id);
        IMGUI_INSPECT_FIELD(user, username);
        IMGUI_INSPECT_FIELD(user, token);
        ImGui::TextDisabled("salt: [HIDDEN]"); // Don't show security-sensitive data
        IMGUI_INSPECT_FIELD(user, email);
        IMGUI_INSPECT_FIELD(user, currentGame);
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const UserLoginAttempt& attempt) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(attempt, username);
        ImGui::TextDisabled("password: [HIDDEN]"); // Don't show passwords
        ImGui::TreePop();
    }
}

inline void InspectStruct(const char* label, const UserResponse& response) {
    if (ImGui::TreeNode(label)) {
        IMGUI_INSPECT_FIELD(response, ok);
        IMGUI_INSPECT_FIELD(response, error);
        if (response.user.has_value()) {
            InspectStruct("user", *response.user);
        } else {
            ImGui::TextDisabled("user: None");
        }
        ImGui::TreePop();
    }
}
