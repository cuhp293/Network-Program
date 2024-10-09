#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

typedef struct {
    int socket;
    char username[USERNAME_SIZE];
} Client;

Client clients[MAX_CLIENTS];

void broadcast_message(int sender_socket, char *message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_socket, max_sd, activity, valread;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    // Initialize client list
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        memset(clients[i].username, 0, USERNAME_SIZE);
    }

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Initialize the file descriptor set
        FD_ZERO(&readfds);  // Monitor standard input
        FD_SET(server_socket, &readfds);    // Monitor the server socket
        max_sd = server_socket;

        // Add all client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        // Use select() to monitor sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        // Handling new connections
        if (FD_ISSET(server_socket, &readfds)) {
            int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            printf("\nNew connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Request username
            char *welcome_msg = "Welcome! Please enter your username: ";
            send(new_socket, welcome_msg, strlen(welcome_msg), 0);

            // Add new client to list
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    printf("Adding client to list of sockets at index %d\n", i);
                    break;
                }
            }
        }

        // Handling I/O from clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    getpeername(sd, (struct sockaddr *)&client_addr, &addr_len);
                    printf("\nClient disconnected, ip %s, port %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // Notify other clients
                    char disconnect_msg[BUFFER_SIZE];
                    snprintf(disconnect_msg, BUFFER_SIZE, "[%s] has left the chat.\n", clients[i].username);
                    broadcast_message(sd, disconnect_msg);

                    close(sd);
                    clients[i].socket = 0;
                    memset(clients[i].username, 0, USERNAME_SIZE);
                } else {
                    buffer[valread] = '\0';
                    
                    if (strlen(clients[i].username) == 0) {
                        // Set username
                        strncpy(clients[i].username, buffer, USERNAME_SIZE - 1);
                        clients[i].username[strcspn(clients[i].username, "\n")] = 0; // Remove newline
                        
                        // Notify other clients
                        char join_msg[BUFFER_SIZE];
                        snprintf(join_msg, BUFFER_SIZE, "[%s] has joined the chat.\n", clients[i].username);
                        broadcast_message(sd, join_msg);
                    } else {
                        // Broadcast message
                        char full_msg[BUFFER_SIZE];
                        snprintf(full_msg, BUFFER_SIZE, "[%s]: %s", clients[i].username, buffer);
                        broadcast_message(sd, full_msg);
                    }
                }
            }
        }
    }

    return 0;
}