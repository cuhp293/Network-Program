#include <stdio.h>

int main() {
    int num;

    // Start the while loop
    printf("Enter numbers (enter a negative number to stop):\n");
    while(1) {  // Infinite loop
        // Input a number
        printf("Enter a number: ");
        scanf("%d", &num);

        // If the number is negative, break the loop
        if (num < 0) {
            break;
        }

        // Print the number if it is non-negative
        printf("You entered: %d\n", num);
    }

    printf("You entered a negative number. Exiting...\n");
    return 0;
}
