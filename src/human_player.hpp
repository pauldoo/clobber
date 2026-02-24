#pragma once

#include "player.hpp"

class HumanPlayer final : public Player {
    
public:
    ~HumanPlayer();

    HumanPlayer();

    Move decide_move(const std::shared_ptr<const GameState>&) override;

    std::string name() const override;

    bool may_attempt_illegal_moves() const override;
};