#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void print_board(int game_board[3][3]) {
    printf("Current game board:\n");
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

int main() {
    int tcp_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    connect(tcp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Connected to the server.\n");

    int game_not_over = 1;
    int game_board[3][3] = {0};

    while (game_not_over) {
        recv(tcp_sock, buffer, BUFFER_SIZE, 0);
        
        if (buffer[0] == 0x05) {
            // TURN_NOTIFICATION
            int valid_move = 0;
            while (!valid_move) {
                int row, col;
                printf("It's your turn. Enter your move (row and column, 0-2): ");
                scanf("%d %d", &row, &col);
                
                buffer[0] = 0x02;  // MOVE message
                buffer[1] = row;
                buffer[2] = col;
                send(tcp_sock, buffer, BUFFER_SIZE, 0);
                
                // Waiting for response from server
                recv(tcp_sock, buffer, BUFFER_SIZE, 0);
                if (buffer[0] == 0x06) {
                    // MOVE_INVALID
                    printf("\nInvalid move! Please try again.\n");
                    print_board(game_board);
                } else {
                    // STATE_UPDATE or RESULT message
                    valid_move = 1;
                    if (buffer[0] == 0x03) {
                        // STATE_UPDATE
                        memcpy(game_board, buffer + 1, sizeof(game_board));
                        print_board(game_board);
                    } else if (buffer[0] == 0x04) {
                        // RESULT
                        if (buffer[1] == 1) {
                            printf("Player 1 wins!\n");
                        } else if (buffer[1] == 2) {
                            printf("Player 2 wins!\n");
                        } else {
                            printf("It's a draw!\n");
                        }
                        game_not_over = 0;
                    }
                }
            }
        } else if (buffer[0] == 0x03) {
            // STATE_UPDATE
            memcpy(game_board, buffer + 1, sizeof(game_board));
            print_board(game_board);
        } else if (buffer[0] == 0x04) {
            // RESULT
            if (buffer[1] == 1) {
                printf("Player 1 wins!\n");
            } else if (buffer[1] == 2) {
                printf("Player 2 wins!\n");
            } else {
                printf("It's a draw!\n");
            }
            game_not_over = 0;
        }
    }
    
    close(tcp_sock);
    return 0;
}
