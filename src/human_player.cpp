#include "human_player.hpp"

#include <type_traits>

static_assert(!std::is_abstract<HumanPlayer>::value, "Must not be abstract");

HumanPlayer::~HumanPlayer() = default;

HumanPlayer::HumanPlayer() = default;

Move HumanPlayer::decide_move(const GameState&) {
    return Move(Location(0, 0), Direction::DOWN);
}
