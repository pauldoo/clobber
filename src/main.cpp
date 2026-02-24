#include <iostream>
#include <memory>

#include "game.hpp"
#include "human_player.hpp"
#include "mcts_player.hpp"
#include "random_player.hpp"

int main(void) {
    std::cout << "Welcome to Clobber!\n\n";

    const long total_games = 30;
    long white_wins = 0;
    for (int i = 0; i < total_games; i++) {
        std::shared_ptr<Player> player1 = std::make_shared<RandomPlayer>();
        std::shared_ptr<Player> player2 = std::make_shared<MCTSPlayer>();

        Game g(player1, player2);

        Side winner = g.run();
        if (winner == Side::WHITE) {
            white_wins += 1;
        }
    }

    std::cout <<
        "White won " << white_wins <<
        " out of " << total_games <<
        " games " <<
        (white_wins*100)/total_games << "%.\n";

    return 0;
}
