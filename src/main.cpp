#include <iostream>
#include <memory>

#include "game.hpp"
#include "human_player.hpp"
#include "random_player.hpp"

int main(void) {
    std::cout << "Welcome to Clobber!\n\n";

    std::shared_ptr<Player> player1 = std::make_shared<HumanPlayer>();
    std::shared_ptr<Player> player2 = std::make_shared<RandomPlayer>();

    Game g(player1, player2);

    g.run();

    return 0;
}
