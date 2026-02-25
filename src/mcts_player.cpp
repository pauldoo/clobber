#include "mcts_player.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <type_traits>

#include "game_state.hpp"
#include "random.hpp"

static_assert(!std::is_abstract<MCTSPlayer>::value, "Must not be abstract");

MCTSPlayer::~MCTSPlayer() = default;

MCTSPlayer::MCTSPlayer(std::ostream& log, long think_iterations_min) :
    m_log(log),
    m_rng(std::make_shared<std::mt19937_64>(make_random_seed())),
    m_think_iterations_min(think_iterations_min)
{
}

std::string MCTSPlayer::name() const {
    std::ostringstream name;
    name << "MCTS(" << m_think_iterations_min << ")";
    return name.str();
}

namespace detail {
    const double UCT_C = std::sqrt(2.0);

    size_t GameStateHash::operator()(const std::shared_ptr<const GameState>& game_state) const {
        return game_state->hash();
    }

    bool GameStateEqual::operator()(
        const std::shared_ptr<const GameState>& lhs,
        const std::shared_ptr<const GameState>& rhs
    ) const {
        return (*lhs) == (*rhs);
    }
    
    class SearchNode final : public std::enable_shared_from_this<SearchNode> {
    private:
        long m_visit_count;
        long m_white_reward;
        bool m_moves_enumerated;

        const std::shared_ptr<const GameState> m_game_state;
        std::vector<std::weak_ptr<SearchNode>> m_parents;
        std::vector<std::shared_ptr<SearchNode>> m_children;
        std::vector<Move> m_moves;
        std::vector<Move> m_untried_moves;

    public:
        ~SearchNode() = default;

        SearchNode(const std::shared_ptr<const GameState>& game_state) :
            m_visit_count(0),
            m_white_reward(0),
            m_moves_enumerated(false),
            m_game_state(game_state) {
        }

        bool is_visited() const {
            return m_visit_count != 0;
        }

        bool is_fully_expanded() const {
            return m_moves_enumerated && m_untried_moves.empty();
        }

        std::shared_ptr<const GameState> game_state() {
            return m_game_state;
        }

        void accumulate_reward_for(Side winner) {
            m_white_reward += (winner == Side::WHITE) ? 1 : 0;
            m_visit_count += 1;
        }

        double white_win_estimate() const {
            return static_cast<double>(m_white_reward) / m_visit_count;
        }

        std::shared_ptr<SearchNode> best_child_by_uct() {
            const long self_visit_count = m_visit_count;
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
                m_white_reward : (m_visit_count - m_white_reward);

            return (static_cast<double>(reward) / m_visit_count) + 
                UCT_C * std::sqrt(std::log(parent_visit_count) / m_visit_count);
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
            return m_visit_count;
        }

        Move best_move(std::ostream& log) const {
            if (!is_fully_expanded()) {
                throw std::runtime_error("Best move not yet known");
            }

            auto best_it = std::max_element(m_children.cbegin(), m_children.cend(),
                [](const std::shared_ptr<SearchNode>& lhs, const std::shared_ptr<SearchNode>& rhs) {
                    return lhs->visit_count() < rhs->visit_count();
                }
            );

            if (best_it == m_children.cend()) {
                throw std::runtime_error("No best move found.");
            }

            std::shared_ptr<SearchNode> best = *best_it;
            double win_estimate = best->white_win_estimate();
            if (m_game_state->next_to_play() == Side::BLACK) {
                win_estimate = 1.0 - win_estimate;
            }

            double decisiveness = static_cast<double>(best->m_visit_count) / m_visit_count;
            log << "Win estimate: " << win_estimate << ", decisiveness: " << decisiveness << "\n";

            auto idx = std::distance(m_children.cbegin(), best_it);
            return m_moves.at(idx);
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
}

Side MCTSPlayer::traverse(std::shared_ptr<detail::SearchNode> node) {
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


Side MCTSPlayer::random_rollout(std::shared_ptr<const GameState> state) const {
    while (true) {
        const std::vector<Move> available_moves = state->all_valid_moves();

        if (available_moves.empty()) {
            return next_side(state->next_to_play());
        }
        std::uniform_int_distribution<int> dist(0, available_moves.size() - 1);
        Move move = available_moves.at(dist(*m_rng));
        state = state->apply_move(move);
    }
}

void MCTSPlayer::update_root(const std::shared_ptr<const GameState>& game_state) {
    size_t size_old = m_node_cache.size();
    m_root_node = detail::find_or_create(m_node_cache, game_state);
    m_node_cache.clear();

    detail::SearchNode::recache(m_node_cache, m_root_node);
    size_t size_new = m_node_cache.size();
    m_log << "Node cache: " << size_old << " -> " << size_new << "\n";
}

Move MCTSPlayer::decide_move(const std::shared_ptr<const GameState>& game_state) {
    update_root(game_state);

    long iteration_count = 0;
    while (true) {
        traverse(m_root_node);
        iteration_count += 1;

        if (iteration_count >= m_think_iterations_min && m_root_node->is_fully_expanded()) {
            break;
        }
    }
    return m_root_node->best_move(m_log);
}
