#include <stdio.h>

int main() {
    char str[100];  // Declare a character array to hold the string

    // Input a string
    printf("Enter a string: ");
    fgets(str, sizeof(str), stdin);  // Using fgets to read the string, including spaces

    // Print the string
    printf("You entered: %s", str);

    return 0;
}
