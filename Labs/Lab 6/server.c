#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_BUFFER 1024
#define MAX_CLIENTS 10
#define XOR_KEY 0x7A

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

typedef struct {
    struct sockaddr_in addr;
    int id;
} Client;

int main() {
    int server_socket;
    struct sockaddr_in server_addr;
    Client clients[MAX_CLIENTS];
    int client_count = 0;
    char buffer[MAX_BUFFER];
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    
    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    printf("Server started. Listening...\n");
    
    while (1) {
        fd_set readfds;
        struct timeval tv;
        
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        
        // Set timeout to 5 seconds
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        int activity = select(server_socket + 1, &readfds, NULL, NULL, &tv);
        
        if (activity < 0) {
            perror("Select error");
            continue;
        }
        
        if (activity == 0) {
            printf("Timeout occurred. No data after 5 seconds.\n");
            continue; // Timeout, continue the loop
        }
        
        if (FD_ISSET(server_socket, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            // Use recvfrom to receive data and the address of the client
            int recv_len = recvfrom(server_socket, buffer, MAX_BUFFER, 0,
                                  (struct sockaddr*)&client_addr, &client_len);
            
            if (recv_len > 0) {
                buffer[recv_len] = '\0';
                
                // Find or add a new client
                int client_index = -1;
                for (int i = 0; i < client_count; i++) {
                    if (client_addr.sin_addr.s_addr == clients[i].addr.sin_addr.s_addr && 
                        client_addr.sin_port == clients[i].addr.sin_port) {
                        client_index = i;
                        break;
                    }
                }
                
                if (client_index == -1 && client_count < MAX_CLIENTS) {
                    client_index = client_count;
                    clients[client_index].addr = client_addr;
                    clients[client_index].id = client_count + 1;
                    client_count++;
                    printf("New client connected. ID: %d, IP: %s, Port: %d\n", 
                           clients[client_index].id,
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                }
                
                if (client_index >= 0) {
                    // Decrypt the received message
                    xor_cipher(buffer, XOR_KEY);
                    printf("Received from client %d: %s\n", clients[client_index].id, buffer);
                    
                    // Prepare response
                    char response[MAX_BUFFER];
                    snprintf(response, MAX_BUFFER, "Client %d sent: %s", 
                            clients[client_index].id, buffer);
                    
                    // Encrypt the response
                    xor_cipher(response, XOR_KEY);
                    
                    // Use sendto to send the response to the specific client
                    if (sendto(server_socket, response, strlen(response), 0,
                             (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
                        perror("Send failed");
                    }
                }
            }
        }
    }
    
    close(server_socket);
    return 0;
}
