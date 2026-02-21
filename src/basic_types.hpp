#pragma once

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <variant>

constexpr int BOARD_WIDTH = 5;
constexpr int BOARD_HEIGHT = 6;

/**
 * The two teams, white and black.
 */
enum class Side : uint8_t {
    WHITE = 0,
    BLACK = 1
};

/**
 * Contents of a square on the board.
 *
 * A white piece, a black peice, or empty.
 */
enum class Square : uint8_t {
    WHITE = static_cast<uint8_t>(Side::WHITE),
    BLACK = static_cast<uint8_t>(Side::BLACK),
    EMPTY
};

/**
 * The four orthogonal directions.
 */
enum class Direction : uint8_t {
    UP,
    DOWN,
    LEFT,
    RIGHT
};



/**
 * A location on the board, just a coordinate.
 */
class Location final {
private:
    uint8_t m_row;
    uint8_t m_column;

public:
    Location(int r, int c);

    uint8_t row() const {
        return m_row;
    }

    uint8_t column() const {
        return m_column;
    }
};

/**
 * Represents a move, basically a start coordinate and a direction.
 */
class Move final {
private:
    Location m_from;
    Direction m_direction;

public:
    Move(Location from, Direction dir);

    Location from() const {
        return m_from;
    }

    Direction direction() const {
        return m_direction;
    }

    Location to() const;
};

Side next_side(Side s);

Square to_square(Side s);

constexpr char square_to_char(Square s);

std::ostream& operator<<(std::ostream& os, Side side);

std::ostream& operator<<(std::ostream& out, Square s);

std::ostream& operator<<(std::ostream& out, Direction d);

std::ostream& operator<<(std::ostream& out, const Location& l);

std::ostream& operator<<(std::ostream& out, const Move& m);

constexpr char square_to_char(Square s) {
    switch (s) {
        case Square::EMPTY:
            return '.';
        case Square::WHITE:
            return 'O';
        case Square::BLACK:
            return 'X';
        default:
            return '?';
    }
}

Direction direction_from_char(char dir);
