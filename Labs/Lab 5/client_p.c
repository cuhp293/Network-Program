#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
        return -1;
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Connect to the server
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Connect error");
        return -1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server...\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client_socket, &readfds);

        // Use select to read from stdin and socket
        select(client_socket + 1, &readfds, NULL, NULL, NULL);

        // Data from stdin
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            send(client_socket, buffer, strlen(buffer), 0);
        }

        // Message from server
        if (FD_ISSET(client_socket, &readfds)) {
            int valread = read(client_socket, buffer, BUFFER_SIZE);
            buffer[valread] = '\0';
            printf("Message from server: %s\n", buffer);
        }
    }

    return 0;
}
