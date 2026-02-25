#include "game.hpp"

#include <ostream>

#include "board.hpp"
#include "game_state.hpp"
#include "player.hpp"

Game::~Game() = default;

Game::Game(
    std::ostream& log,
    const std::shared_ptr<Player>& p1,
    const std::shared_ptr<Player>& p2
) : m_log(log)
{
    Board initial_board;
    initial_board.set_to_initial_state();

    m_game_state = std::make_shared<const GameState>(
        Side::WHITE, initial_board
    );
    m_players.at(0) = p1;
    m_players.at(1) = p2;
}

Side Game::run() {
    while (true) {
        m_log << std::string(20, '=') << "\n";
        m_log << *m_game_state;
        if (!m_game_state->can_continue()) {
            break;
        }

        do_next_turn();
    }
    m_log << std::string(20, '=') << "\n";
    Side winner = next_side(m_game_state->next_to_play());
    m_log
        << "Game finished\n"
        << "Winner: " << winner << "\n";
    
    return winner;
}

void Game::do_next_turn() {
    auto player = m_players.at(static_cast<size_t>(m_game_state->next_to_play()));
    m_log << "\n" << player->name() << "'s turn\n";

    const Move m = player->decide_move(m_game_state);
    m_log << "Move: " << m << "\n";

    try {
        m_game_state = m_game_state->apply_move(m);
    } catch (const std::exception& e) {
        if (player->may_attempt_illegal_moves()) {
            m_log << "That move is illegal. Please try again.\n";
        } else {
            throw;
        }
    }
}

