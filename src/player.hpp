#pragma once

#include "basic_types.hpp"
#include "forwards.hpp"

class Player {
public:
    virtual ~Player() = 0;

    virtual Move decide_move(const GameState&) = 0;

    virtual std::string name() = 0;
};

