#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int n;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Answer the questions:\n");

    // Answer questions
    for (int i = 0; i < 10; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (n <= 0) {
            break;
        }
        
        printf("%s", buffer);
        printf("Enter your answer (1-4): ");

        char answer[10];
        fgets(answer, sizeof(answer), stdin);
        send(client_socket, answer, strlen(answer), 0);
    }

    // Get points from server
    memset(buffer, 0, BUFFER_SIZE);
    n = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (n > 0) {
        printf("%s", buffer);
    }

    // Close the socket
    close(client_socket);

    return 0;
}
