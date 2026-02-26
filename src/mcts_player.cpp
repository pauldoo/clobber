#include "mcts_player.hpp"

#include <algorithm>
#include <future>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <thread>
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

    size_t GameStateHash::operator()(const std::shared_ptr<const GameState>& game_state) const {
        return game_state->hash();
    }

    bool GameStateEqual::operator()(
        const std::shared_ptr<const GameState>& lhs,
        const std::shared_ptr<const GameState>& rhs
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
    
    class SearchNode final : public std::enable_shared_from_this<SearchNode> {
    private:
        NodeStats m_stats;
        bool m_moves_enumerated;

        const std::shared_ptr<const GameState> m_game_state;
        std::vector<std::weak_ptr<SearchNode>> m_parents;
        std::vector<std::shared_ptr<SearchNode>> m_children;
        std::vector<Move> m_moves;
        std::vector<Move> m_untried_moves;

    public:
        ~SearchNode() = default;

        SearchNode(const std::shared_ptr<const GameState>& game_state) :
            m_moves_enumerated(false),
            m_game_state(game_state) {
        }

        bool is_visited() const {
            return m_stats.m_visit_count != 0;
        }

        bool is_fully_expanded() const {
            return m_moves_enumerated && m_untried_moves.empty();
        }

        std::shared_ptr<const GameState> game_state() {
            return m_game_state;
        }

        void accumulate_reward_for(Side winner) {
            m_stats.m_white_reward += (winner == Side::WHITE) ? 1 : 0;
            m_stats.m_visit_count += 1;
        }

        std::shared_ptr<SearchNode> best_child_by_uct() {
            const long self_visit_count = m_stats.m_visit_count;
            const Side self_side = m_game_state->next_to_play();
            auto max_it = std::max_element(m_children.begin(), m_children.end(),
                [&](const std::shared_ptr<SearchNode>& lhs, const std::shared_ptr<SearchNode>& rhs) {
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

        std::shared_ptr<SearchNode> expand(NodeCache& cache) {
            if (!m_moves_enumerated) {
                m_untried_moves = m_game_state->all_valid_moves();
                m_moves_enumerated = true;
            }
            std::shared_ptr<SearchNode> self = shared_from_this();
            if (m_untried_moves.empty()) {
                // This can happen if the game is over.
                return nullptr;
            }

            Move move = m_untried_moves.back();
            m_untried_moves.pop_back();
            std::shared_ptr<const GameState> new_state = m_game_state->apply_move(move);
            std::shared_ptr<SearchNode> child_node = find_or_create(cache, new_state);
    
            m_children.push_back(child_node);
            child_node->m_parents.push_back(self);
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
            const std::vector<std::shared_ptr<const SearchNode>>& root_nodes
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

        static void recache(
            NodeCache& cache,
            const std::shared_ptr<detail::SearchNode>& node
        ) {
            auto insert = cache.insert({node->game_state(), node});
            if (insert.first->first != node->game_state()) {
                throw std::runtime_error("Key mismatch.");
            }
            if (insert.first->second != node) {
                throw std::runtime_error("Value mismatch.");
            }
            if (insert.second) {
                // Node was newly inserted, so recache children too.
                for (auto& child : node->m_children) {
                    recache(cache, child);
                }
            }
        }

    };

    std::shared_ptr<SearchNode> find_or_create(
        NodeCache& cache,
        const std::shared_ptr<const GameState>& game_state
    ) {
        auto it = cache.find(game_state);
        if (it != cache.end()) {
            return it->second;
        } else {
            std::shared_ptr<SearchNode> node = std::make_shared<SearchNode>(game_state);
            auto insert = cache.emplace(game_state, node);
            if (!insert.second) {
                throw std::runtime_error("Insertion failed.");
            }
            return insert.first->second;
        }
    }

    class SingleSearch final {
    private:
        detail::NodeCache m_node_cache;
        std::shared_ptr<detail::SearchNode> m_root_node;
        std::mt19937_64 m_rng;

        Side traverse(std::shared_ptr<detail::SearchNode> node);

        Side random_rollout(std::shared_ptr<const GameState> state);

        void update_root(const std::shared_ptr<const GameState>& game_state);

    public:
        SingleSearch();

        std::shared_ptr<const detail::SearchNode> evaluate_moves(
            const std::shared_ptr<const GameState>& game_state,
            long think_iterations_min
        );
    };

    SingleSearch::SingleSearch() : m_rng(make_random_seed()) {
    }

    Side SingleSearch::traverse(std::shared_ptr<detail::SearchNode> node) {
        std::shared_ptr<detail::SearchNode> next;
        if (node->is_visited() && !node->is_terminal()) {
            if (node->is_fully_expanded()) {
                next = node->best_child_by_uct();
            } else {
                next = node->expand(m_node_cache);
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

    Side SingleSearch::random_rollout(std::shared_ptr<const GameState> state) {
        while (true) {
            const std::vector<Move> available_moves = state->all_valid_moves();

            if (available_moves.empty()) {
                return next_side(state->next_to_play());
            }
            std::uniform_int_distribution<int> dist(0, available_moves.size() - 1);
            Move move = available_moves.at(dist(m_rng));
            state = state->apply_move(move);
        }
    }

    void SingleSearch::update_root(const std::shared_ptr<const GameState>& game_state) {
        m_root_node = detail::find_or_create(m_node_cache, game_state);
        m_node_cache.clear();

        detail::SearchNode::recache(m_node_cache, m_root_node);
    }

    std::shared_ptr<const detail::SearchNode> SingleSearch::evaluate_moves(
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
        std::vector<std::future<std::shared_ptr<const detail::SearchNode>>> futures;
        for (detail::SingleSearch& s: m_searchers) {
            futures.push_back(std::async(std::launch::async,
                [&s, game_state, think_iterations_min]() {
                    return s.evaluate_moves(game_state, think_iterations_min);
                }
            ));
        }

        std::vector<std::shared_ptr<const detail::SearchNode>> results;
        for (auto& f : futures) {
            results.push_back(f.get());
        }

        return detail::SearchNode::best_move(m_log, results);
    }

}

Move MCTSPlayer::decide_move(const std::shared_ptr<const GameState>& game_state) {
    return m_search->decide_move(game_state, m_think_iterations_min);
}
