#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "Message from client";
    char buffer[BUFFER_SIZE] = {0};
    char server_ip[INET_ADDRSTRLEN];

    // Creating socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    // Enter server's IP address
    printf("Enter server's IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP addresses: string to binary
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address or address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        return -1;
    }

    // Send a message to server
    write(sock, message, strlen(message));

    // Read the server's response
    read(sock, buffer, BUFFER_SIZE);
    printf("Server response: %s\n", buffer);

    // Close the socket
    close(sock);

    return 0;
}