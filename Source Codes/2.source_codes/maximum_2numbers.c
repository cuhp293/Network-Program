#include <stdio.h>

int main() {
    float num1, num2;

    // Input two float numbers
    printf("Enter two float numbers: ");
    scanf("%f %f", &num1, &num2);

    // Compare the two numbers
    if (num1 > num2) {
        printf("The maximum number is: %.2f\n", num1);
    } else if (num2 > num1) {
        printf("The maximum number is: %.2f\n", num2);
    } else {
        printf("Both numbers are equal.\n");
    }

    return 0;
}
