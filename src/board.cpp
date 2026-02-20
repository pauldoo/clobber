#include "board.hpp"

#include <ostream>
#include <stdexcept>

Board::~Board() = default;

Board::Board() = default;

size_t Board::index(Location loc) const {
    return loc.row() * BOARD_WIDTH + loc.column();
}

Square Board::get_square(Location loc) const {
    return m_grid.at(index(loc));
}

void Board::set_square(Location loc, Square s) {
    m_grid.at(index(loc)) = s;
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