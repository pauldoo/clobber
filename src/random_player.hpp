#pragma once

#include <random>

#include "player.hpp"

class RandomPlayer final : public Player {
private:
    std::mt19937_64 m_rng;

public:
    ~RandomPlayer();

    RandomPlayer();

    Move decide_move(const std::shared_ptr<const GameState>&) override;

    std::string name() const override;

    bool may_attempt_illegal_moves() const override;
};

