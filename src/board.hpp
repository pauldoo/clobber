#pragma once

#include <array>
#include <iosfwd>
#include <cstdint>

#include "basic_types.hpp"

class Board final
{
public:

    std::array<std::array<Square, BOARD_WIDTH>, BOARD_HEIGHT> m_grid;

public:
    ~Board();

    Board();

    Square get_square(Location l) const;

    void set_square(Location l, Square s);

    void set_to_initial_state();

    size_t hash() const;
};

bool operator==(const Board&, const Board&);

std::ostream& operator<<(std::ostream& out, const Board& b);
