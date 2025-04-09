#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <string>
#include <cstdlib> // Para rand() e srand()
#include <ctime> // Para time()
#include <vector>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() {
        // Inicializar o tabuleiro e as variáveis do jogo
        for (auto& row : board) {
            row.fill(' ');
        }
        current_player = 'X'; 
        game_over = false; 
        winner = ' '; 
    }

    void display_board() {
        // Exibir o tabuleiro no console
        std::cout << "Tabuleiro:\n";
        for (const auto& row : board) {
            for (const auto& cell : row) {
                std::cout << cell << " ";
            }
            std::cout << "\n";
        }
        std::cout << "Jogador atual: " << current_player << "\n";
    }

    bool make_move(char player, int row, int col) {
        // Implementar a lógica para realizar uma jogada no tabuleiro
        // Utilizar mutex para controle de acesso
        // Utilizar variável de condição para alternância de turnos
        std::unique_lock<std::mutex> lock(board_mutex); //Garante que só uma thread altere o tabuleiro por vez e protege as variáveis compartilhadas, como current_player e game_over
        turn_cv.wait(lock, [this, player] {return current_player == player || game_over;}); // Espera a vez do jogador ou o término do jogo
        if (player != current_player) {
            std::cerr << "Não é a vez do jogador " << player << "!\n";
            return false; // Não é a vez do jogador
        }
        if (game_over || board[row][col] != ' ') {
            return false; // Jogada inválida; posição já ocupada ou jogo terminou
        }
        board[row][col] = player; 
        display_board(); 

        if(check_win(player)){
            game_over = true;
            winner = player;
        }else if(check_draw()){
            game_over = false;
            winner = 'D'; // Empate
        }else{
            current_player = (current_player == 'X') ? 'O' : 'X'; // Alternar jogador
        }

        turn_cv.notify_all(); 
        return true; // Jogada válida
    }

    bool check_win(char player) {
        // Verificar se o jogador atual venceu o jogo
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
                return true;  // Venceu na linha ou coluna
            }
        }
        if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
            (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
            return true; // Venceu na diagonal
        }
        return false; 
    }

    bool check_draw() {
        // Verificar se houve um empate
        for (const auto& row : board) {
            for (const auto& cell : row) {
                if (cell == ' ') {
                    return false; // Ainda há jogadas disponíveis
                }
            }
        }
        return true; // Empate
    }

    bool is_game_over() {
        // Retornar se o jogo terminou
        std::unique_lock<std::mutex> lock(board_mutex); 
        return game_over; 
    }

    char get_winner() {
        // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
        std::unique_lock<std::mutex> lock(board_mutex);
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
         while (!game.is_game_over()){
            if(strategy == "sequential"){
                play_sequential(); 
            }else if(strategy == "random"){
                play_random();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
         }

         char result = game.get_winner();
         if(result == symbol){
            std::cout << "Jogador " << symbol << " venceu!\n"; 
            } else if(result == 'D'){
            std::cout << "Empate!\n";
            }
    }
    

private:
    void play_sequential() {
        // Implementar a estratégia sequencial de jogadas
        for(int row = 0; row < 3; row++){
            for(int col = 0; col <3; col++){
                if(game.make_move(symbol, row, col)){
                    return;
                }
            }
        }
    }

    void play_random() {
        // Implementar a estratégia aleatória de jogadas
       while(true){
            int row = rand() % 3;
            int col = rand() % 3;
            if(game.make_move(symbol, row, col)){
            return; // Jogada válida
            }
       }
    }
};

// Função principal
int main() {
    srand(time(0)); 

    TicTacToe game; 
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    std::thread t1(&Player::play, &player1); 
    std::thread t2(&Player::play, &player2);

    t1.join(); 
    t2.join();

    game.display_board(); 
    char winner = game.get_winner();
    if(winner == 'D'){
        std::cout << "Resultado final: Empate!\n";
    }else{
        std::cout << "Resultado final: Jogador " << winner << " venceu!\n";
    }
    return 0; 
}
