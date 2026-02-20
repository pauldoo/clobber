#include "basic_types.hpp"

#include <ostream>


Location::Location(int r, int c) : m_row(r), m_column(c) {
    if (!(r >= 0 && r < BOARD_HEIGHT && c >= 0 && c < BOARD_WIDTH)) {
        throw std::out_of_range("Invalid location");
    }
}

Move::Move(Location from, Direction dir) :
    m_from(from), m_direction(dir)
{
    bool is_valid = false;
    switch (m_direction) {
        case Direction::UP:
            is_valid = m_from.row() > 0;
            break;

        case Direction::DOWN:
            is_valid = m_from.row() < (BOARD_HEIGHT-1);
            break;

        case Direction::LEFT:
            is_valid = m_from.column() > 0;
            break;

        case Direction::RIGHT:
            is_valid = m_from.column() < (BOARD_WIDTH-1);
            break;

        default:
            throw std::out_of_range("Invalid direction");
    }
    if (!is_valid) {
        throw std::out_of_range("Illegal move (off the board)");
    }
}

Location Move::to() const {
    switch (m_direction) {
        case Direction::UP:
            return {m_from.row()-1, m_from.column()};
        case Direction::DOWN:
            return {m_from.row()+1, m_from.column()};
        case Direction::LEFT:
            return {m_from.row(), m_from.column()-1};
        case Direction::RIGHT:
            return {m_from.row(), m_from.column()+1};
        default:
            throw std::out_of_range("Invalid direction");            
    }
}

std::ostream& operator<<(std::ostream& out, Side side) {
    switch (side) {
        case Side::WHITE:
            out << "white";
            break;
        case Side::BLACK:
            out << "black";
            break;
    }
    out << "(" << to_square(side) << ")";
    return out;
}

std::ostream& operator<<(std::ostream& out, Square s) {
    out << square_to_char(s);
    return out;
}


Square to_square(Side s) {
    switch (s) {
        case Side::WHITE:
            return Square::WHITE;
        case Side::BLACK:
            return Square::BLACK;
        default:
            throw std::out_of_range("Invalid side");

    }
}

Side next_side(Side s) {
    switch (s) {
        case Side::WHITE:
           return Side::BLACK;
        case Side::BLACK:
            return Side::WHITE;
        default:
            throw std::out_of_range("Invalid side");            
    }
}
