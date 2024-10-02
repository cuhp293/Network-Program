#include <stdio.h>

int main() {
    unsigned int num1, num2;

    // Input two integers
    printf("Enter two integers: ");
    scanf("%u %u", &num1, &num2);

    // Bitwise AND
    printf("Bitwise AND of %u and %u is: %u\n", num1, num2, num1 & num2);

    // Bitwise OR
    printf("Bitwise OR of %u and %u is: %u\n", num1, num2, num1 | num2);

    // Bitwise XOR
    printf("Bitwise XOR of %u and %u is: %u\n", num1, num2, num1 ^ num2);

    // Bitwise NOT of num1
    printf("Bitwise NOT of %u is: %u\n", num1, ~num1);

    // Left shift num1 by 2 positions
    printf("%u left shifted by 2 positions is: %u\n", num1, num1 << 2);

    // Right shift num2 by 2 positions
    printf("%u right shifted by 2 positions is: %u\n", num2, num2 >> 2);

    return 0;
}
