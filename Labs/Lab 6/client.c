#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_BUFFER 1024
#define XOR_KEY "XorKey2024" // Longer key for better encryption
#define SERVER_TIMEOUT 10  // Timeout for server response in seconds
#define MAX_RETRIES 3      // Maximum number of retries for sending a message

void xor_cipher(char *data, size_t data_len, const char *key) {
    size_t key_len = strlen(key);
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len];
    }
}

int send_with_timeout(int sock, const char *msg, size_t len, int flags, 
                      struct sockaddr *dest_addr, socklen_t addrlen, int timeout_sec) {
    fd_set writefds;
    struct timeval tv;
    int retries = 0;

    while (retries < MAX_RETRIES) {
        FD_ZERO(&writefds);
        FD_SET(sock, &writefds);

        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;

        int ready = select(sock + 1, NULL, &writefds, NULL, &tv);

        if (ready == -1) {
            if (errno == EINTR) continue;  // Interrupted, try again
            perror("select() error");
            return -1;
        } else if (ready == 0) {
            printf("Send timeout occurred. Retrying... (%d/%d)\n", retries + 1, MAX_RETRIES);
            retries++;
            continue;
        }

        if (FD_ISSET(sock, &writefds)) {
            return sendto(sock, msg, len, flags, dest_addr, addrlen);
        }
    }

    printf("Max retries reached. Message not sent.\n");
    return -1;
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
    
    printf("Connected to server. Type your messages (type 'exit' to quit):\n");
    
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        int max_fd = (client_socket > STDIN_FILENO) ? client_socket : STDIN_FILENO;
        
        struct timeval tv;
        tv.tv_sec = SERVER_TIMEOUT;
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (activity < 0) {
            perror("Select error");
            continue;
        }
        
        if (activity == 0) {
            printf("No activity for %d seconds. Server might be unresponsive.\n", SERVER_TIMEOUT);
            continue;
        }
        
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(input, MAX_BUFFER, stdin) != NULL) {
                input[strcspn(input, "\n")] = 0;
                
                if (strcmp(input, "exit") == 0) {
                    printf("Closing...\n");
                    break;
                }
                
                size_t input_len = strlen(input);
                memcpy(buffer, input, input_len);
                xor_cipher(buffer, input_len, XOR_KEY);
                
                if (send_with_timeout(client_socket, buffer, input_len, 0,
                                      (struct sockaddr*)&server_addr, sizeof(server_addr), 5) < 0) {
                    printf("Failed to send message after multiple attempts.\n");
                    continue;
                }
                printf("Sent encrypted message to server\n");
            }
        }
        
        if (FD_ISSET(client_socket, &readfds)) {
            int bytes_received = recvfrom(client_socket, buffer, MAX_BUFFER, 0, NULL, NULL);
            
            if (bytes_received > 0) {
                xor_cipher(buffer, bytes_received, XOR_KEY);
                buffer[bytes_received] = '\0'; // Null-terminate after decryption
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