#include "human_player.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <type_traits>

static_assert(!std::is_abstract<HumanPlayer>::value, "Must not be abstract");

HumanPlayer::~HumanPlayer() = default;

HumanPlayer::HumanPlayer(std::istream& in, std::ostream& out) :
    m_in(in), m_out(out)
{
}

std::string HumanPlayer::name() const {
    return "Human";
}

bool HumanPlayer::may_attempt_illegal_moves() const {
    return true;
}

Move HumanPlayer::decide_move(const std::shared_ptr<const GameState>&) {
    while (true) {
        m_out <<
            "Your move. Please enter: <row> <col> <direction>\n" <<
            "> " << std::flush;

        m_in.clear();
        int row, column;
        char direction;
        m_in >> row >> column >> direction;
        if (!m_in) {
            if (m_in.eof()) {
                throw std::runtime_error("Game aborted.");
            }
            m_out << "Oops, didn't get that. Try again.\n";
            continue;
        }

        try {
            return Move{{row, column}, direction_from_char(direction)};
        } catch (const std::exception& e) {
            m_out << "Oops, " << e.what() << "\n";
        }
    }
}

