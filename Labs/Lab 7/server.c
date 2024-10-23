#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int game_board[3][3];  // 3x3 game board
int move_count = 0;    // Number of moves made

void init_board() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            game_board[i][j] = 0;  // Initialize the board (0 means empty)
}

void print_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            printf("%d ", game_board[i][j]);
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
    
    return 0;  // No winner
}

void notify_turn(int client_sock) {
    char buffer[BUFFER_SIZE] = {0x05};  // 0x05 is the TURN_NOTIFICATION message
    send(client_sock, buffer, BUFFER_SIZE, 0);
}

void send_state_update(int client1_sock, int client2_sock) {
    char buffer[BUFFER_SIZE] = {0x03};  // 0x03 is the STATE_UPDATE message
    memcpy(buffer + 1, game_board, sizeof(game_board));
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
}

void send_result(int client1_sock, int client2_sock, int result) {
    char buffer[BUFFER_SIZE] = {0x04};  // 0x04 is the RESULT message
    buffer[1] = result;
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
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

        if (current_player == 1) {
            notify_turn(client1_sock);  // Notify Player 1's turn
            recv(client1_sock, buffer, BUFFER_SIZE, 0);  // Receive move from Player 1
        } else {
            notify_turn(client2_sock);  // Notify Player 2's turn
            recv(client2_sock, buffer, BUFFER_SIZE, 0);  // Receive move from Player 2
        }

        // Get row and column from buffer
        row = buffer[1];
        col = buffer[2];

        // Validate the move
        if (game_board[row][col] == 0) {
            game_board[row][col] = current_player;
            move_count++;

            // Check for winner or draw
            int winner = check_winner();
            if (winner > 0 || move_count == 9) {
                send_result(client1_sock, client2_sock, winner);  // Send result
                game_not_over = 0;
            } else {
                send_state_update(client1_sock, client2_sock);  // Update the board
            }
            
            // Switch turns
            current_player = (current_player == 1) ? 2 : 1;
        }
    }
    
    close(client1_sock);
    close(client2_sock);
    close(server_sock);
    return 0;
}
