#include "random_player.hpp"

#include <type_traits>

#include "game_state.hpp"
#include "random.hpp"

static_assert(!std::is_abstract<RandomPlayer>::value, "Must not be abstract");

RandomPlayer::~RandomPlayer() = default;

RandomPlayer::RandomPlayer() : m_rng(make_random_seed()) {
}

std::string RandomPlayer::name() const {
    return "Random";
}

Move RandomPlayer::decide_move(const GameState& game_state) {
    auto available_moves = game_state.all_valid_moves();

    std::uniform_int_distribution<int> dist(0, available_moves.size() - 1);
    return available_moves.at(dist(m_rng));
}

