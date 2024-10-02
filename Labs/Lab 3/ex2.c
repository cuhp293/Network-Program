#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    char ip_str[INET_ADDRSTRLEN];
    printf("Enter IP address (as a string): ");
    fgets(ip_str, INET_ADDRSTRLEN, stdin);
    ip_str[strcspn(ip_str, "\n")] = '\0';

    struct in_addr ip_addr;

    // Convert the IP address from string to binary form using inet_pton
    if (inet_pton(AF_INET, ip_str, &ip_addr) == 1) {
        // printf("inet_pton: Successfully converted IP address: %s\n", ip_str);
    } else {
        printf("inet_pton: Failed to convert IP address: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    char ip_str_converted[INET_ADDRSTRLEN];  // Buffer to hold the converted IP address back to string

    // Convert the binary IPv4 address back to string form
    if (inet_ntop(AF_INET, &ip_addr, ip_str_converted, INET_ADDRSTRLEN)) {
        printf("inet_ntop: Converted back to string IP address: %s\n", ip_str_converted);
    } else {
        printf("inet_ntop: Failed to convert IP address back to string\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
