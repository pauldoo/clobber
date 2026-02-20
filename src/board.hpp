#pragma once

#include <array>
#include <iosfwd>
#include <cstdint>

#include "basic_types.hpp"

class Board final
{
public:

    std::array<Square, BOARD_WIDTH * BOARD_HEIGHT> m_grid;

    size_t index(Location l) const;

public:
    ~Board();

    Board();

    Square get_square(Location l) const;

    void set_square(Location l, Square s);

    void set_to_initial_state();
};

std::ostream& operator<<(std::ostream& out, const Board& b);
