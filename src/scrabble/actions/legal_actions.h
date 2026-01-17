#pragma once

#include <vector>
#include <string>
#include <optional>

namespace scrabble {
    struct Word;
    struct GameState;

    struct WordCounter {
        [[nodiscard]] bool ContainsKey(char c) const;
        void InsertOrIncrement(char c);
        [[nodiscard]] int Get(char c) const;
        [[nodiscard]] std::vector<char> Keys() const;
        void Subtract(char c, int k);

    private:
        int map_[26] = {};
    };

    bool can_steal_word(const std::string &new_word, const Word& stolen_word, const GameState &game);

    std::optional<int> get_tile_by_id(const std::string &id, const GameState &game);
}