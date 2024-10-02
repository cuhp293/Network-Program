#include <stdio.h>

int main() {
    int n, sum = 0;

    // Input a positive integer
    printf("Enter a positive integer: ");
    scanf("%d", &n);

    // Use for loop to calculate the sum of first n natural numbers
    for(int i = 1; i <= n; i++) {
        sum += i;
    }

    // Print the result
    printf("The sum of the first %d natural numbers is: %d\n", n, sum);

    return 0;
}
