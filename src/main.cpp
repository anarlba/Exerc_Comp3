#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>

class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board;
    std::mutex board_mutex;
    std::condition_variable turn_cv;
    char current_player;
    bool game_over;
    char winner;

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' ') {
        for (auto& row : board) {
            row.fill(' ');
        }
    }

    void display_board() {
        std::cout << "\nTabuleiro:\n";
        for (const auto& row : board) {
            for (const auto& cell : row) {
                std::cout << cell << " ";
            }
            std::cout << "\n";
        }
        std::cout << "Jogador atual: " << current_player << "\n\n";
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);

        if (game_over || current_player != player || row < 0 || row > 2 || col < 0 || col > 2 || board[row][col] != ' ') {
            return false;
        }

        board[row][col] = player;
        display_board();

        if (check_win(player)) {
            game_over = true;
            winner = player;
        } else if (check_draw()) {
            game_over = true;
            winner = 'D';
        } else {
            current_player = (current_player == 'X') ? 'O' : 'X';
        }

        turn_cv.notify_all();
        return true;
    }

    bool is_my_turn(char player) {
        std::unique_lock<std::mutex> lock(board_mutex);
        return !game_over && current_player == player;
    }

    bool is_game_over() {
        std::unique_lock<std::mutex> lock(board_mutex);
        return game_over;
    }

    char get_winner() {
        std::unique_lock<std::mutex> lock(board_mutex);
        return winner;
    }

private:
    bool check_win(char player) {
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
                return true;
            }
        }
        return (board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
               (board[0][2] == player && board[1][1] == player && board[2][0] == player);
    }

    bool check_draw() {
        for (const auto& row : board) {
            for (const auto& cell : row) {
                if (cell == ' ') {
                    return false;
                }
            }
        }
        return true;
    }
};

class Player {
    private:
        TicTacToe& game;
        char symbol;
        std::string strategy;
    
    public:
        Player(TicTacToe& g, char s, std::string strat)
            : game(g), symbol(s), strategy(strat) {}
    
        void play() {
            while (!game.is_game_over()) {
                if (game.is_my_turn(symbol)) {
                    if (strategy == "sequential") {
                        play_sequential();
                    } else if (strategy == "random") {
                        play_random();
                    }
                    // Delay de 500ms após a jogada
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                } else {
                    // Espera um pouco antes de tentar de novo
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
    
    private:
        void play_sequential() {
            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    if (game.make_move(symbol, row, col)) return;
                }
            }
        }
    
        void play_random() {
            while (!game.is_game_over()) {
                int row = rand() % 3;
                int col = rand() % 3;
                if (game.make_move(symbol, row, col)) return;
            }
        }
    };

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    TicTacToe game;
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    std::cout << "Iniciando jogo da velha concorrente...\n";
    std::cout << "Jogador X: estratégia sequencial\n";
    std::cout << "Jogador O: estratégia aleatória\n";

    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    t1.join();
    t2.join();

    char winner = game.get_winner();
    if (winner == 'D') {
        std::cout << "\nResultado final: Empate!\n";
    } else {
        std::cout << "\nResultado final: Jogador " << winner << " venceu!\n";
    }

    return 0;
}