#include "human_player.hpp"

#include <stdexcept>
#include <iostream>
#include <type_traits>

static_assert(!std::is_abstract<HumanPlayer>::value, "Must not be abstract");

HumanPlayer::~HumanPlayer() = default;

HumanPlayer::HumanPlayer() = default;

std::string HumanPlayer::name() const {
    return "Human";
}

bool HumanPlayer::may_attempt_illegal_moves() const {
    return true;
}

Move HumanPlayer::decide_move(const std::shared_ptr<const GameState>&) {
    while (true) {
        std::cout <<
            "Your move. Please enter: <row> <col> <direction>\n" <<
            "> " << std::flush;

        std::cin.clear();
        int row, column;
        char direction;
        std::cin >> row >> column >> direction;
        if (!std::cin) {
            if (std::cin.eof()) {
                throw std::runtime_error("Game aborted.");
            }
            std::cout << "Oops, didn't get that. Try again.\n";
            continue;
        }

        try {
            return Move{{row, column}, direction_from_char(direction)};
        } catch (const std::exception& e) {
            std::cout << "Oops, " << e.what() << "\n";
        }
    }
}

