#pragma once

#include <iosfwd>
#include <unordered_map>

#include "player.hpp"

namespace detail {
    class ParallelSearch;
}

class MCTSPlayer final : public Player {
private:
    std::ostream& m_log;
    const long m_think_iterations_min;
    const int m_thread_count;

    std::unique_ptr<detail::ParallelSearch> m_search;

public:
    ~MCTSPlayer();

    MCTSPlayer(std::ostream& log, long think_iterations_min, int thread_count);

    Move decide_move(const std::shared_ptr<const GameState>&) override;

    std::string name() const override;

    bool may_attempt_illegal_moves() const override {
        return false;
    }

};
