#pragma once

#include <iosfwd>

#include "player.hpp"

class HumanPlayer final : public Player {
private:
    std::istream& m_in;
    std::ostream& m_out;
    
public:
    ~HumanPlayer();

    HumanPlayer(std::istream& in, std::ostream& out);

    Move decide_move(const std::shared_ptr<const GameState>&) override;

    std::string name() const override;

    bool may_attempt_illegal_moves() const override;
};
