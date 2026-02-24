#include "board.hpp"

#include <ostream>
#include <stdexcept>

Board::~Board() = default;

Board::Board() = default;

Square Board::get_square(Location loc) const {
    return m_grid.at(loc.row()).at(loc.column());
}

void Board::set_square(Location loc, Square s) {
    m_grid.at(loc.row()).at(loc.column()) = s;
}

void Board::set_to_initial_state() {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            Square s =  ((row + col) % 2) == 0 ?
                Square::WHITE :
                Square::BLACK;
            set_square(Location(row, col), s);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const Board& b) {
    out << " ";
    for (int col = 0; col < BOARD_WIDTH; col++) {
        out << " " << col;
    }
    out << "\n\n";
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        out << row;
        for (int col = 0; col < BOARD_WIDTH; col++) {
            out << " " << b.get_square(Location(row, col));
        }
        out << "\n";
    }
    return out;
}

bool operator==(const Board& lhs, const Board& rhs) {
    return lhs.m_grid == rhs.m_grid;
}

size_t Board::hash() const {
    size_t result = 0;
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            result = (result * 3) + static_cast<size_t>(m_grid.at(row).at(col));
        }
    }
    return result;
}