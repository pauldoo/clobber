#pragma once

#include <iosfwd>
#include <random>
#include <unordered_map>

#include "player.hpp"

namespace detail {
    class SearchNode;

    struct GameStateHash {
        size_t operator()(const std::shared_ptr<const GameState>& game_state) const;
    };

    struct GameStateEqual {
        bool operator()(
            const std::shared_ptr<const GameState>& lhs,
            const std::shared_ptr<const GameState>& rhs
        ) const;
    };

    using NodeCache = std::unordered_map<
        std::shared_ptr<const GameState>,
        std::shared_ptr<SearchNode>,
        GameStateHash,
        GameStateEqual>;

    std::shared_ptr<SearchNode> find_or_create(
        NodeCache& cache,
        const std::shared_ptr<const GameState>& game_state
    );        
}

class MCTSPlayer final : public Player {
private:
    std::ostream& m_log;
    std::shared_ptr<std::mt19937_64> m_rng;
    const long m_think_iterations_min;

    detail::NodeCache m_node_cache;
    std::shared_ptr<detail::SearchNode> m_root_node;

    Side traverse(std::shared_ptr<detail::SearchNode> node);

    Side random_rollout(std::shared_ptr<const GameState> state) const;

    void update_root(const std::shared_ptr<const GameState>& game_state);

public:
    ~MCTSPlayer();

    MCTSPlayer(std::ostream& log, long think_iterations_min);

    Move decide_move(const std::shared_ptr<const GameState>&) override;

    std::string name() const override;

    bool may_attempt_illegal_moves() const override {
        return false;
    }

};
