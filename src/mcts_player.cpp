#include "mcts_player.hpp"

#include <algorithm>
#include <future>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "game_state.hpp"
#include "random.hpp"

static_assert(!std::is_abstract<MCTSPlayer>::value, "Must not be abstract");

MCTSPlayer::~MCTSPlayer() = default;

MCTSPlayer::MCTSPlayer(std::ostream& log, long think_iterations_min, int thread_count) :
    m_log(log),
    m_think_iterations_min(think_iterations_min),
    m_thread_count(thread_count)
{
    m_search = std::make_unique<detail::ParallelSearch>(m_log, m_thread_count);
}

std::string MCTSPlayer::name() const {
    std::ostringstream name;
    name << "MCTS(" << m_thread_count << " x " << m_think_iterations_min << ")";
    return name.str();
}

namespace detail {
    const double UCT_C = std::sqrt(2.0);

    class SearchNode;

    using CacheKey = const GameState*;
    using CacheValue = std::unique_ptr<SearchNode>;

    struct GameStateHash {
        size_t operator()(const CacheKey& game_state) const;
    };

    struct GameStateEqual {
        bool operator()(
            const CacheKey& lhs,
            const CacheKey& rhs
        ) const;
    };


    using NodeCache = std::unordered_map<
        CacheKey,
        CacheValue,
        GameStateHash,
        GameStateEqual>;

    SearchNode* find_or_create(
        std::vector<std::unique_ptr<const GameState>>& states,
        NodeCache& cache,
        std::unique_ptr<const GameState> game_state
    );

    size_t GameStateHash::operator()(const CacheKey& game_state) const {
        return game_state->hash();
    }

    bool GameStateEqual::operator()(
        const CacheKey& lhs,
        const CacheKey& rhs
    ) const {
        return (*lhs) == (*rhs);
    }

    struct NodeStats {
        long m_visit_count = 0;
        long m_white_reward = 0;
    };

    NodeStats& operator+=(NodeStats& lhs, const NodeStats& rhs) {
        lhs.m_visit_count += rhs.m_visit_count;
        lhs.m_white_reward += rhs.m_white_reward;
        return lhs;
    }
    
    class SearchNode final {
    private:
        NodeStats m_stats;
        bool m_moves_enumerated;

        // Game state is owned by the node cache, so we use raw pointers within the graph.
        const GameState* m_game_state;
        // Children are owned by the node cache, so we use raw pointers within the graph.
        std::vector<SearchNode*> m_children;
        std::vector<Move> m_moves;
        std::vector<Move> m_untried_moves;

    public:
        ~SearchNode() = default;

        SearchNode(const GameState* game_state) :
            m_moves_enumerated(false),
            m_game_state(game_state) {
        }

        bool is_visited() const {
            return m_stats.m_visit_count != 0;
        }

        bool is_fully_expanded() const {
            return m_moves_enumerated && m_untried_moves.empty();
        }

        const GameState* game_state() {
            return m_game_state;
        }

        void accumulate_reward_for(Side winner) {
            m_stats.m_white_reward += (winner == Side::WHITE) ? 1 : 0;
            m_stats.m_visit_count += 1;
        }

        SearchNode* best_child_by_uct() {
            const long self_visit_count = m_stats.m_visit_count;
            const Side self_side = m_game_state->next_to_play();
            auto max_it = std::max_element(m_children.begin(), m_children.end(),
                [&](SearchNode* lhs, SearchNode* rhs) {
                    return lhs->uct_value(self_visit_count, self_side) < rhs->uct_value(self_visit_count, self_side);
                }
            );

            if (max_it == m_children.end()) {
                throw std::runtime_error("Failed to find max child by UCT.");
            }
            return *max_it;
        }

        double uct_value(long parent_visit_count, Side parent_side) const {
            if (!is_visited()) {
                throw std::runtime_error("UCT unavailable, node not visited.");
            }

            long reward = (parent_side == Side::WHITE) ?
                m_stats.m_white_reward : (m_stats.m_visit_count - m_stats.m_white_reward);

            return (static_cast<double>(reward) / m_stats.m_visit_count) + 
                UCT_C * std::sqrt(std::log(parent_visit_count) / m_stats.m_visit_count);
        }

        bool is_terminal() const {
            return m_moves_enumerated && m_children.empty() && m_untried_moves.empty();
        }

        SearchNode* expand(std::vector<std::unique_ptr<const GameState>>& states, NodeCache& cache) {
            if (!m_moves_enumerated) {
                m_untried_moves = m_game_state->all_valid_moves();
                m_moves_enumerated = true;
            }
            if (m_untried_moves.empty()) {
                // This can happen if the game is over.
                return nullptr;
            }

            Move move = m_untried_moves.back();
            m_untried_moves.pop_back();
            std::unique_ptr<const GameState> new_state = m_game_state->apply_move(move);
            SearchNode* child_node = find_or_create(states, cache, std::move(new_state));
    
            m_children.push_back(child_node);
            m_moves.push_back(move);

            if (m_untried_moves.empty()) {
                // That was the last move to expand, so compact memory.
                m_untried_moves.shrink_to_fit();
                m_moves.shrink_to_fit();
                m_children.shrink_to_fit();
            }

            return child_node;
        }

        long visit_count() const {
            return m_stats.m_visit_count;
        }

        static Move best_move(
            std::ostream& log,
            const std::vector<const detail::SearchNode*>& root_nodes
        ) {
            long root_visit_count = 0;
            std::vector<Move> avalable_moves = (*root_nodes.cbegin())->m_moves;
            std::vector<NodeStats> move_stats(avalable_moves.size());

            for (auto& root: root_nodes) {
                if (!root->is_fully_expanded()) {
                    throw std::runtime_error("Root not expanded.");
                }
                if (avalable_moves != root->m_moves) {
                    throw std::runtime_error("Available moves don't match.");
                }

                root_visit_count += root->visit_count();
                for (size_t i = 0; i < avalable_moves.size(); i++) {
                    move_stats.at(i) += root->m_children.at(i)->m_stats;
                }
            }

            const auto best_it = std::max_element(
                    move_stats.cbegin(), move_stats.cend(),
                    [](const NodeStats& lhs, const NodeStats& rhs) {
                        return lhs.m_visit_count < rhs.m_visit_count;
                    });

            double win_estimate = static_cast<double>(best_it->m_white_reward) / best_it->m_visit_count;
            if ((*root_nodes.cbegin())->m_game_state->next_to_play() == Side::BLACK) {
                win_estimate = 1.0 - win_estimate;
            }

            double decisiveness = static_cast<double>(best_it->m_visit_count) / root_visit_count;
            log << "Win estimate: " << win_estimate << ", decisiveness: " << decisiveness << ", total samples: " << root_visit_count << "\n";

            return avalable_moves.at(std::distance(move_stats.cbegin(), best_it));
        }

        static void recache_impl(
            NodeCache& old_cache,
            NodeCache& new_cache,
            detail::SearchNode* const node
        ) {
            auto new_it = new_cache.find(node->game_state());
            if (new_it != new_cache.end()) {
                if (new_it->second.get() != node) {
                    throw std::runtime_error("State already in cache with different value.");
                }
                return;
            }

            auto old_it = old_cache.find(node->game_state());
            if (old_it == old_cache.end()) {
                throw std::runtime_error("State not present in old cache to migrate.");
            }
            std::unique_ptr<SearchNode> node_ptr = std::move(old_it->second);
            if (node_ptr.get() != node) {
                throw std::runtime_error("Node cache didn't point to me.");
            };
            new_cache.emplace(node->game_state(), std::move(node_ptr));

            for (detail::SearchNode* child : node->m_children) {
                recache_impl(old_cache, new_cache, child);
            }
        }

        static void recache(
            std::vector<std::unique_ptr<const GameState>>& states,
            NodeCache& cache,
            detail::SearchNode* const node
        ) {
            NodeCache old_cache = std::move(cache);

            recache_impl(old_cache, cache, node);

            auto new_end = std::remove_if(states.begin(), states.end(),
                [&](const std::unique_ptr<const GameState>& ptr) {
                    return cache.find(ptr.get()) == cache.end();
                }
            );
            states.erase(new_end, states.end());
        }
    };

    SearchNode* find_or_create(
        std::vector<std::unique_ptr<const GameState>>& states,
        NodeCache& cache,
        std::unique_ptr<const GameState> game_state
    ) {
        auto it = cache.find(game_state.get());
        if (it != cache.end()) {
            return it->second.get();
        } else {
            const GameState* game_state_raw = game_state.get();
            states.push_back(std::move(game_state));
            std::unique_ptr<SearchNode> node = std::make_unique<SearchNode>(game_state_raw);
            auto insert = cache.emplace(game_state_raw, std::move(node));
            if (!insert.second) {
                throw std::runtime_error("Insertion failed.");
            }
            return insert.first->second.get();
        }
    }

    class SingleSearch final {
    private:
        std::vector<std::unique_ptr<const GameState>> m_states;
        detail::NodeCache m_node_cache;
        detail::SearchNode* m_root_node;
        std::mt19937_64 m_rng;

        Side traverse(detail::SearchNode* node);

        Side random_rollout(const GameState* state);

        void update_root(const std::shared_ptr<const GameState>& game_state);

    public:
        SingleSearch();

        const detail::SearchNode* evaluate_moves(
            const std::shared_ptr<const GameState>& game_state,
            long think_iterations_min
        );
    };

    SingleSearch::SingleSearch() : m_rng(make_random_seed()) {
    }

    Side SingleSearch::traverse(detail::SearchNode* node) {
        detail::SearchNode* next = nullptr;
        if (node->is_visited() && !node->is_terminal()) {
            if (node->is_fully_expanded()) {
                next = node->best_child_by_uct();
            } else {
                next = node->expand(m_states, m_node_cache);
            }
        }

        Side winner;
        if (next) {
            winner = traverse(next);
        } else {
            winner = random_rollout(node->game_state());
        }
        
        node->accumulate_reward_for(winner);
        return winner;
    }  

    Side SingleSearch::random_rollout(const GameState* state) {
        // First iteration is special, since we do not own the starting state.
        const std::vector<Move> available_moves = state->all_valid_moves();

        if (available_moves.empty()) {
            return next_side(state->next_to_play());
        }
        std::uniform_int_distribution<int> dist(0, available_moves.size() - 1);
        Move move = available_moves.at(dist(m_rng));

        std::unique_ptr<GameState> new_state = state->apply_move(move);

        // Subsequent iterations must own the states.
        while (true) {
            const std::vector<Move> available_moves = new_state->all_valid_moves();

            if (available_moves.empty()) {
                return next_side(new_state->next_to_play());
            }
            std::uniform_int_distribution<int> dist(0, available_moves.size() - 1);
            Move move = available_moves.at(dist(m_rng));
            new_state = new_state->apply_move(move);
        }
    }

    void SingleSearch::update_root(const std::shared_ptr<const GameState>& game_state) {
        m_root_node = detail::find_or_create(m_states, m_node_cache, std::make_unique<GameState>(*game_state));

        detail::SearchNode::recache(m_states, m_node_cache, m_root_node);
    }

    const detail::SearchNode* SingleSearch::evaluate_moves(
        const std::shared_ptr<const GameState>& game_state,
        long think_iterations_min
    ) {
        update_root(game_state);

        long iteration_count = 0;
        while (true) {
            traverse(m_root_node);
            iteration_count += 1;

            if (iteration_count >= think_iterations_min && m_root_node->is_fully_expanded()) {
                break;
            }
        }

        return m_root_node;
    }

    class ParallelSearch final {
    private:
        std::ostream& m_log;
        std::vector<detail::SingleSearch> m_searchers;

    public:
        ParallelSearch(std::ostream& log, int thread_count);

        Move decide_move(
            const std::shared_ptr<const GameState>& game_state,
            long think_iterations_min
        );
    };

    ParallelSearch::ParallelSearch(std::ostream& log, int thread_count) : m_log(log) {
        m_searchers.resize(thread_count);
    }

    Move ParallelSearch::decide_move(
        const std::shared_ptr<const GameState>& game_state,
        long think_iterations_min
    ) {
        std::vector<std::future<const detail::SearchNode*>> futures;
        for (detail::SingleSearch& s: m_searchers) {
            futures.push_back(std::async(std::launch::async,
                [&s, game_state, think_iterations_min]() {
                    return s.evaluate_moves(game_state, think_iterations_min);
                }
            ));
        }

        std::vector<const detail::SearchNode*> results;
        for (auto& f : futures) {
            results.push_back(f.get());
        }

        return detail::SearchNode::best_move(m_log, results);
    }

}

Move MCTSPlayer::decide_move(const std::shared_ptr<const GameState>& game_state) {
    return m_search->decide_move(game_state, m_think_iterations_min);
}
