#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

char username[USERNAME_SIZE];

void *receive_messages(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);
    }

    if (read_size == 0) {
        puts("Server disconnected");
    } else if (read_size == -1) {
        perror("recv failed");
    }

    return 0;
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    pthread_t receive_thread;

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

    // Receive and respond to username prompt
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);
    fgets(username, USERNAME_SIZE, stdin);
    send(client_socket, username, strlen(username), 0);

    // Create a thread to receive messages
    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&client_socket) < 0) {
        perror("Could not create thread");
        return -1;
    }

    // Main loop to send messages
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
    return 0;
}