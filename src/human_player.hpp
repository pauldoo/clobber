#pragma once

#include "player.hpp"

class HumanPlayer final : public Player {
public:
    ~HumanPlayer();

    HumanPlayer();

    Move decide_move(const GameState&) override;
};