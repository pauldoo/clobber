#include "game_state.hpp"

#include <ostream>
#include <stdexcept>

#include "board.hpp"

GameState::~GameState() = default;

GameState::GameState(const GameState&) = default;

GameState::GameState(Side next_to_play, Board board) :
    m_next_to_play(next_to_play), m_board(board) {
}

void GameState::apply_move_inplace(const Move m) {
    if (
        m_board.get_square(m.from()) == to_square(m_next_to_play) &&
        m_board.get_square(m.to()) == to_square(next_side(m_next_to_play))
    ) {
        m_board.set_square(m.from(), Square::EMPTY);
        m_board.set_square(m.to(), to_square(m_next_to_play));
        m_next_to_play = next_side(m_next_to_play);
    } else {
        throw std::invalid_argument("Invalid move!");
    }
}

std::unique_ptr<GameState> GameState::apply_move(const Move m) const {
    std::unique_ptr<GameState> result = std::make_unique<GameState>(*this);

    result->apply_move_inplace(m);

    return result;
}

bool GameState::can_continue() const {
    Square expected_from = to_square(m_next_to_play);
    Square expected_to = to_square(next_side(m_next_to_play)); 

    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (m_board.get_square({row, col}) == expected_from) {

                if (
                    (row > 0 && m_board.get_square({row-1, col}) == expected_to) ||
                    (row < (BOARD_HEIGHT-1) && m_board.get_square({row+1, col}) == expected_to) ||
                    (col > 0 && m_board.get_square({row, col-1}) == expected_to) ||
                    (col < (BOARD_WIDTH-1) && m_board.get_square({row, col+1}) == expected_to)
                ) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& out, const GameState& s) {
    out << s.board() << "\n";
    out << "Next to play: " << s.next_to_play() << "\n";
    return out;
}

std::vector<Move> GameState::all_valid_moves() const {
    Square expected_from = to_square(m_next_to_play);
    Square expected_to = to_square(next_side(m_next_to_play)); 

    std::vector<Move> result;
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (m_board.get_square({row, col}) == expected_from) {

                if (row > 0 && m_board.get_square({row-1, col}) == expected_to) {
                    result.push_back(Move{{row, col}, Direction::UP});
                }
                if (row < (BOARD_HEIGHT-1) && m_board.get_square({row+1, col}) == expected_to) {
                    result.push_back(Move{{row, col}, Direction::DOWN});
                }
                if (col > 0 && m_board.get_square({row, col-1}) == expected_to) {
                    result.push_back(Move{{row, col}, Direction::LEFT});
                }
                if (col < (BOARD_WIDTH-1) && m_board.get_square({row, col+1}) == expected_to) {
                    result.push_back(Move{{row, col}, Direction::RIGHT});
                }
            }
        }
    }

    return result;
}

size_t GameState::hash() const {
    return static_cast<int>(m_next_to_play) * 31 + m_board.hash();
}

bool operator==(const GameState& lhs, const GameState& rhs) {
    return lhs.next_to_play() == rhs.next_to_play() && lhs.board() == rhs.board();
}