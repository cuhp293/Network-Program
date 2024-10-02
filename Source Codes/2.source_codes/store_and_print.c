#include <stdio.h>

int main() {
    int numbers[5];  // Declare an array of size 5

    // Input 5 integers
    printf("Enter 5 integers:\n");
    for(int i = 0; i < 5; i++) {
        printf("Enter number %d: ", i + 1);
        scanf("%d", &numbers[i]);
    }

    // Print the integers
    printf("You entered the following numbers:\n");
    for(int i = 0; i < 5; i++) {
        printf("%d ", numbers[i]);
    }

    printf("\n");
    return 0;
}
