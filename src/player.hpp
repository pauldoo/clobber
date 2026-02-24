#pragma once

#include <string>

#include "basic_types.hpp"
#include "forwards.hpp"

class Player {
public:
    virtual ~Player() = 0;

    virtual Move decide_move(const GameState&) = 0;

    virtual std::string name() const = 0;

    /**
     * Returns true if the player might accidentally suggest an illegal move.
     */
    virtual bool may_attempt_illegal_moves() const = 0;
};

