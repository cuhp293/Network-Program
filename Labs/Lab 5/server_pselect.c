#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

volatile sig_atomic_t waiter = 0;

void sigint_handler(int signo) {
    waiter = 1;
}

void broadcast_message(int *client_sockets, int sender_sd, char *message, int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        int sd = client_sockets[i];
        if (sd != 0 && sd != sender_sd) {
            send(sd, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_socket, new_socket;
    int client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[BUFFER_SIZE] = {0};
    int addrlen = sizeof(address);
    struct timespec timeout;
    sigset_t sigmask;

    // Set up signal handling
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
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

    // Initialize timeout and signal mask
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);

    while(!waiter) {
        // Clear the socket set
        FD_ZERO(&readfds);
        
        // Add server socket to set
        FD_SET(server_socket, &readfds);
        int max_sd = server_socket;
        
        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if(sd > 0) {
                FD_SET(sd, &readfds);
            }
            if(sd > max_sd) {
                max_sd = sd;
            }
        }
        
        // Wait for an activity on one of the sockets, using pselect()
        int activity = pselect(max_sd + 1, &readfds, NULL, NULL, &timeout, &sigmask);
        
        if (activity < 0 && errno != EINTR) {
            perror("pselect error");
            continue;
        }
        
        // If something happened on the server socket, then it's an incoming connection
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            // Add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if(client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    printf("\nNew connection, socket fd is %d, ip is: %s, port: %d\n", 
                           new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    break;
                }
            }
        }
        
        // Check for I/O operation on client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            
            if (FD_ISSET(sd, &readfds)) {
                int valread;
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Client disconnected
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Client disconnected, ip %s, port %d\n", 
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_sockets[i] = 0;
                }
                else {
                    // Broadcast the message to all other clients
                    buffer[valread] = '\0';
                    broadcast_message(client_sockets, sd, buffer, MAX_CLIENTS);
                }
            }
        }
    }

    // Close all sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > 0) {
            close(client_sockets[i]);
        }
    }
    close(server_socket);
    
    printf("\nServer shutting down.\n");

    return 0;
}
