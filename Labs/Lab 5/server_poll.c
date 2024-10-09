#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void broadcast_message(struct pollfd *fds, int sender_fd, char *message, int nfds) {
    for (int i = 1; i < nfds; i++) {
        if (fds[i].fd != sender_fd && fds[i].fd != -1) {
            send(fds[i].fd, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};
    int addrlen = sizeof(address);
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;

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

    // Add server socket to poll
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    while(1) {
        int activity = poll(fds, nfds, -1);
        
        if (activity < 0) {
            perror("poll error");
            exit(EXIT_FAILURE);
        }
        
        // Check for events on server socket (new connection)
        if (fds[0].revents & POLLIN) {
            if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            printf("\nNew connection, socket fd is %d, ip is: %s, port: %d\n", 
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            
            // Add new socket to array of sockets
            if (nfds < MAX_CLIENTS + 1) {
                fds[nfds].fd = new_socket;
                fds[nfds].events = POLLIN;
                nfds++;
            } else {
                printf("Too many connections, connection rejected\n");
                close(new_socket);
            }
        }
        
        // Check for I/O operation on client sockets
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                int valread;
                if ((valread = read(fds[i].fd, buffer, BUFFER_SIZE)) == 0) {
                    // Client disconnected
                    getpeername(fds[i].fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Client disconnected, ip %s, port %d\n", 
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                else {
                    // Broadcast the message to all other clients
                    buffer[valread] = '\0';
                    broadcast_message(fds, fds[i].fd, buffer, nfds);
                }
            }
        }
        
        // Remove disconnected clients from the array
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) {
                for(int j = i; j < nfds - 1; j++) {
                    fds[j] = fds[j+1];
                }
                nfds--;
                i--;
            }
        }
    }

    return 0;
}
