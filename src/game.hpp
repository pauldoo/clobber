#pragma once

#include <array>
#include <iosfwd>
#include <memory>

#include "basic_types.hpp"
#include "forwards.hpp"

/**
 * A game being played, contains the state of the game and the players.
 */
class Game {
private:
    std::ostream& m_log;
    std::array<std::shared_ptr<Player>, 2> m_players;
    std::shared_ptr<const GameState> m_game_state;

public:
    ~Game();

    Game(
        std::ostream& log,
        const std::shared_ptr<Player>& p1,
        const std::shared_ptr<Player>& p2
    );

    /** returns the winner. */
    Side run();
    void do_next_turn();
};

