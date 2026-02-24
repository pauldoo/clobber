#pragma once

#include <iosfwd>
#include <memory>
#include <vector>

#include "basic_types.hpp"
#include "board.hpp"

class GameState final {
private:
    Side m_next_to_play;
    Board m_board;
public:
    ~GameState();

    explicit GameState(const GameState&);

    GameState(Side next_to_play, Board board);

    /**
     * The next player cannot move.
     */
    bool can_continue() const;

    Side next_to_play() const {
        return m_next_to_play;
    }

    const Board& board() const {
        return m_board;
    }

    void apply_move_inplace(const Move m);

    std::unique_ptr<GameState> apply_move(const Move m) const;

    std::vector<Move> all_valid_moves() const;
};

std::ostream& operator<<(std::ostream&, const GameState&);