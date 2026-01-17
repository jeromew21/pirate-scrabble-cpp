#include "legal_actions.h"
#include "legal_actions.h"
#include "legal_actions.h"

#include "scrabble/context/types.h"

using namespace scrabble;

bool WordCounter::ContainsKey(const char c) const {
    return map_[c - 65] > 0;
}

void WordCounter::InsertOrIncrement(const char c) {
    map_[c - 65]++;
}

int WordCounter::Get(const char c) const {
    return map_[c - 65];
}

std::vector<char> WordCounter::Keys() const {
    auto result = std::vector<char>();
    for (char i = 'A'; i <= 'Z'; i++) {
        if (ContainsKey(i)) result.push_back(i);
    }
    return result;
}

void WordCounter::Subtract(const char c, const int k) {
    map_[c - 65] -= k;
}

bool scrabble::can_steal_word(const std::string &new_word, const Word &stolen_word, const GameState &game) {
    for (auto &s: stolen_word.history) {
        if (s == new_word) return false;
    }

    // first check if *all letters* in stolenWord are in use
    auto stolenCounter = WordCounter{};
    auto wordCounter = WordCounter{};
    for (const auto c: new_word) {
        wordCounter.InsertOrIncrement(c);
    }
    for (const auto c: stolen_word.history.front()) {
        stolenCounter.InsertOrIncrement(c);
    }

    for (auto key : stolenCounter.Keys()) {
        if (!wordCounter.ContainsKey(key)) return false;
        wordCounter.Subtract(key, stolenCounter.Get(key));
        if (wordCounter.Get(key) < 0) return false;
    }

    auto publicCounter = WordCounter{};

    for (auto &t : game.tiles)
    {
        if (!t.faceUp) continue;
        const char c = t.letter.front();
        publicCounter.InsertOrIncrement(c);
    }

    for (const auto key : wordCounter.Keys())
    {
        if (wordCounter.Get(key) == 0) continue;
        //if (!publicCounter.ContainsKey(key)) return false;
        if (publicCounter.Get(key) < wordCounter.Get(key)) {
            return false;
        }
    }

    return true;
}

std::optional<int> scrabble::get_tile_by_id(const std::string &id, const GameState &game) {
    int i = 0;
    for (TileProps t : game.tiles ) {
        if (id == t.id) return i;
        i++;
    }
    return std::nullopt;
}
