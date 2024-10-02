#include <stdio.h>

int main() {
    // Use for loop to iterate through numbers less than 100
    printf("Odd integers less than 100 are:\n");
    for(int i = 1; i < 100; i += 2) {
        printf("%d ", i);
    }

    printf("\n");
    return 0;
}
