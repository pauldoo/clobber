#include "random_player.hpp"

#include <type_traits>

#include "random.hpp"

static_assert(!std::is_abstract<RandomPlayer>::value, "Must not be abstract");

RandomPlayer::~RandomPlayer() = default;

RandomPlayer::RandomPlayer() : m_rng(make_random_seed()) {
    std::uniform_int_distribution<int> uniform_dist(1, 6);
}

std::string RandomPlayer::name() const {
    return "Random";
}

Move RandomPlayer::decide_move(const GameState&) {
    return Move(Location(0, 0), Direction::DOWN);
}

