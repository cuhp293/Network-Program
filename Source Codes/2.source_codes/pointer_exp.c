#include <stdio.h>

int main() {
    int num = 10;  // Declare an integer variable
    int *ptr;      // Declare a pointer to an integer

    // Assign the address of num to the pointer
    ptr = &num;

    // Display the value of num and the pointer information
    printf("Value of num: %d\n", num);
    printf("Address of num: %p\n", &num);
    printf("Value stored in ptr (address of num): %p\n", ptr);
    printf("Value pointed to by ptr (value of num): %d\n", *ptr);

    // Modify the value of num using the pointer
    *ptr = 20;

    // Display the modified value of num
    printf("\nAfter modifying num through the pointer:\n");
    printf("Value of num: %d\n", num);

    return 0;
}
