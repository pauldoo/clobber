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

Direction direction_from_char(char dir) {
    Direction d;
    switch (dir) {
        case 'u':
        case 'U':
            d = Direction::UP;
            break;
        
        case 'd':
        case 'D':
            d = Direction::DOWN;
            break;

        case 'l':
        case 'L':
            d = Direction::LEFT;
            break;

        case 'r':
        case 'R':
            d = Direction::RIGHT;
            break;

        default:
            throw std::out_of_range("Unrecognised direction");
    }
    return d;
}

std::ostream& operator<<(std::ostream& out, Direction d) {
    switch (d) {
        case Direction::UP:
            out << 'U';
            break;
        case Direction::DOWN:
            out << 'D';
            break;
        case Direction::LEFT:
            out << 'L';
            break;
        case Direction::RIGHT:
            out << 'R';
            break;
        default:
            throw std::out_of_range("Invalid direction");
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Location& l) {
    out << static_cast<int>(l.row()) << " " << static_cast<int>(l.column());
    return out;
}

std::ostream& operator<<(std::ostream& out, const Move& m) {
    out << m.from() << " " << m.direction();
    return out;
}

bool operator==(const Move& lhs, const Move& rhs) {
    return lhs.from() == rhs.from() && lhs.direction() == rhs.direction();
}

bool operator==(const Location& lhs, const Location& rhs) {
    return lhs.row() == rhs.row() && lhs.column() == rhs.column();
}