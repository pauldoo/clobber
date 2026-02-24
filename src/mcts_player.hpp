#pragma once

#include <random>

#include "player.hpp"

class MCTSPlayer final : public Player {
private:
    std::shared_ptr<std::mt19937_64> m_rng;
    const long m_think_iterations_min;
    
public:
    ~MCTSPlayer();

    MCTSPlayer(long think_iterations_min);

    Move decide_move(const std::shared_ptr<const GameState>&) override;

    std::string name() const override;

    bool may_attempt_illegal_moves() const override {
        return false;
    }

};
