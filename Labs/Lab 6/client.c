#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_BUFFER 1024
#define XOR_KEY 0x7A

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER];
    char input[MAX_BUFFER];
    
    // Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);
    
    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    printf("Connected to server. Type your messages (type 'exit' to quit):\n");
    
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        int max_fd = (client_socket > STDIN_FILENO) ? client_socket : STDIN_FILENO;
        
        // Select will wait until there is data from the server or from the keyboard
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        
        if (activity < 0) {
            perror("Select error");
            continue;
        }
        
        // Check if there is input from the keyboard
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(input, MAX_BUFFER, stdin) != NULL) {
                // Remove newline character at the end
                input[strcspn(input, "\n")] = 0;
                
                // Check if the user wants to exit
                if (strcmp(input, "exit") == 0) {
                    printf("Closing connection...\n");
                    break;
                }
                
                // Encrypt the message before sending
                strcpy(buffer, input);
                xor_cipher(buffer, XOR_KEY);
                
                // Use send() since we have already connected
                if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
                    perror("Send failed");
                    continue;
                }
                printf("Sent encrypted message to server\n");
            }
        }
        
        // Check if there is data from the server
        if (FD_ISSET(client_socket, &readfds)) {
            // Use recv() since we have already connected
            int bytes_received = recv(client_socket, buffer, MAX_BUFFER, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                
                // Decrypt and display the message
                xor_cipher(buffer, XOR_KEY);
                printf("Server response: %s\n", buffer);
            }
            else if (bytes_received < 0) {
                perror("Receive failed");
            }
        }
    }
    
    close(client_socket);
    return 0;
}
