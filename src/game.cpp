#include "game.hpp"

#include <iostream>

#include "board.hpp"
#include "game_state.hpp"
#include "player.hpp"

Game::~Game() = default;

Game::Game(
    const std::shared_ptr<Player>& p1,
    const std::shared_ptr<Player>& p2
) {
    Board initial_board;
    initial_board.set_to_initial_state();

    m_game_state = std::make_unique<const GameState>(
        Side::WHITE, initial_board
    );
    m_players.at(0) = p1;
    m_players.at(1) = p2;
}

void Game::run() {
    while (true) {
        std::cout << "----------\n";
        std::cout << *m_game_state << "\n";
        if (!m_game_state->can_continue()) {
            break;
        }

        do_next_turn();
    }
    std::cout
        << "Game finished\n"
        << "Winner: " << next_side(m_game_state->next_to_play()) << "\n";
}

void Game::do_next_turn() {
    auto player = m_players.at(static_cast<size_t>(m_game_state->next_to_play()));
    std::cout << player->name() << "'s turn\n";

    const Move m = player->decide_move(*m_game_state);
    std::cout << "Move: " << m << "\n";

    try {
        m_game_state = m_game_state->apply_move(m);
    } catch (const std::exception& e) {
        if (player->may_attempt_illegal_moves()) {
            std::cout << "That move is illegal. Please try again.\n";
        } else {
            throw;
        }
    }
}

