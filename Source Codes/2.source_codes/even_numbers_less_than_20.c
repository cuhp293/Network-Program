#include <stdio.h>

int main() {
    int i = 0;

    // Use while loop to iterate through numbers less than 20
    printf("Even numbers less than 20 are:\n");
    while(i < 20) {
        i++;  // Increment the counter

        // If the number is odd, skip the rest of the loop
        if (i % 2 != 0) {
            continue;  // Skip the rest of the loop if the number is odd
        }

        // Print the even number
        printf("%d ", i);
    }

    printf("\n");
    return 0;
}
