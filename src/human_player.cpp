#include "human_player.hpp"

#include <iostream>
#include <type_traits>

static_assert(!std::is_abstract<HumanPlayer>::value, "Must not be abstract");

HumanPlayer::~HumanPlayer() = default;

HumanPlayer::HumanPlayer() = default;

std::string HumanPlayer::name() const {
    return "Human";
}

Move HumanPlayer::decide_move(const GameState&) {
    while (true) {
        std::cout <<
            "Your move. Please enter: <row> <col> <direction>\n" <<
            "> " << std::flush;

        std::cin.clear();
        int row, column;
        char direction;
        std::cin >> row >> column >> direction;
        if (!std::cin) {
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

