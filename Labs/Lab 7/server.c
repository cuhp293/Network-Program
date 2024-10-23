#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int game_board[3][3];
int move_count = 0;

void init_board() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            game_board[i][j] = 0;  // 0 means empty
}

void print_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            char symbol = '.';  // Empty board
            if (game_board[i][j] == 1) {
                symbol = 'x';   // Player 1
            } else if (game_board[i][j] == 2) {
                symbol = 'o';   // Player 2
            }
            printf("%c ", symbol);
        }
        printf("\n");
    }
}

// Check for a winner
int check_winner() {
    // Check rows, columns, and diagonals
    for (int i = 0; i < 3; i++) {
        if (game_board[i][0] == game_board[i][1] && game_board[i][1] == game_board[i][2] && game_board[i][0] != 0)
            return game_board[i][0];
        if (game_board[0][i] == game_board[1][i] && game_board[1][i] == game_board[2][i] && game_board[0][i] != 0)
            return game_board[0][i];
    }
    if (game_board[0][0] == game_board[1][1] && game_board[1][1] == game_board[2][2] && game_board[0][0] != 0)
        return game_board[0][0];
    if (game_board[0][2] == game_board[1][1] && game_board[1][1] == game_board[2][0] && game_board[0][2] != 0)
        return game_board[0][2];
    
    // No winner
    return 0;
}

// TURN_NOTIFICATION message
void notify_turn(int client_sock) {
    char buffer[BUFFER_SIZE] = {0x05};
    send(client_sock, buffer, BUFFER_SIZE, 0);
}

// STATE_UPDATE message
void send_state_update(int client1_sock, int client2_sock) {
    char buffer[BUFFER_SIZE] = {0x03};
    memcpy(buffer + 1, game_board, sizeof(game_board));
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
}

// RESULT message
void send_result(int client1_sock, int client2_sock, int result) {
    char buffer[BUFFER_SIZE] = {0x04};
    buffer[1] = result;
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
}

// INVALID MOVE message
void notify_invalid_move(int client_sock) {
    char buffer[BUFFER_SIZE] = {0x06};
    send(client_sock, buffer, BUFFER_SIZE, 0);
}

int main() {
    int server_sock, client1_sock, client2_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 2);
    
    printf("Waiting players to connect...\n");
    client1_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    printf("Player 1 connected.\n");
    client2_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    printf("Player 2 connected.\n");

    init_board();
    int current_player = 1;
    int game_not_over = 1;

    while (game_not_over) {
        int row, col;
        char buffer[BUFFER_SIZE] = {0};
        int current_client_sock = (current_player == 1) ? client1_sock : client2_sock;

        notify_turn(current_client_sock);  // Notify current player's turn
        
        int valid_move = 0;
        while (!valid_move) {
            recv(current_client_sock, buffer, BUFFER_SIZE, 0);
            row = buffer[1];
            col = buffer[2];
            
            // Check if MOVE is valid
            if (row < 0 || row >= 3 || col < 0 || col >= 3) {
                notify_invalid_move(current_client_sock);
                continue;
            }
            
            if (game_board[row][col] != 0) {
                notify_invalid_move(current_client_sock);
                continue;
            }
            
            valid_move = 1;
        }

        game_board[row][col] = current_player;
        move_count++;

        // Check for winner or draw
        int winner = check_winner();
        if (winner > 0 || move_count == 9) {
            send_result(client1_sock, client2_sock, winner);
            game_not_over = 0;
        } else {
            send_state_update(client1_sock, client2_sock);
        }
        
        current_player = (current_player == 1) ? 2 : 1;
    }
    
    close(client1_sock);
    close(client2_sock);
    close(server_sock);
    return 0;
}
