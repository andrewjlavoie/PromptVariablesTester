#include "complex_macros.h"
#include <stdio.h>

void swap_example(int* a, int* b) {
    printf("Before swap: a = %d, b = %d\n", *a, *b);
    SWAP(*a, *b, int);
    printf("After swap: a = %d, b = %d\n", *a, *b);
}

IntArray* create_int_array(size_t size) {
    IntArray* array = INIT_FLEXIBLE_ARRAY(int, IntArray, size);
    CHECK_ALLOC(array);
    
    array->size = size;
    
    // Initialize array values
    for (size_t i = 0; i < size; i++) {
        array->data[i] = i * 10;
    }
    
    return array;
}

void debug_example(void) {
#ifdef DEBUG
    printf("Debug mode is enabled\n");
    
    int x = 5;
    int y = 10;
    
    DEBUG_PRINT("x = %d, y = %d\n", x, y);
    DEBUG_PRINT("MAX(x, y) = %d\n", MAX(x, y));
    DEBUG_PRINT("MIN(x, y) = %d\n", MIN(x, y));
    
    ASSERT(x < y, "x should be less than y");
    
    // This assertion would fail and abort the program if uncommented
    // ASSERT(x > y, "This should fail");
#else
    printf("Debug mode is disabled\n");
#endif
}

// Example using FOR_EACH macro
void print_array(int* array, size_t length) {
    printf("Array: ");
    int item;
    FOR_EACH(item, array, length) {
        printf("%d ", item);
    }
    printf("\n");
}

// Example of using ERROR_RETURN
int divide_safely(int a, int b, int* result) {
    ERROR_RETURN(b == 0, -1, "Error: Division by zero\n");
    ERROR_RETURN(result == NULL, -2, "Error: NULL result pointer\n");
    
    *result = a / b;
    return 0;
}

// Main function to test the macros
int main() {
    // Test swap macro
    int a = 5, b = 10;
    swap_example(&a, &b);
    
    // Test array creation
    IntArray* array = create_int_array(5);
    printf("Created array with %zu elements: ", array->size);
    for (size_t i = 0; i < array->size; i++) {
        printf("%d ", array->data[i]);
    }
    printf("\n");
    
    // Test FOR_EACH macro
    int numbers[] = {1, 2, 3, 4, 5};
    print_array(numbers, 5);
    
    // Test debug macros
    debug_example();
    
    // Test safe division
    int result;
    if (divide_safely(10, 2, &result) == 0) {
        printf("10 / 2 = %d\n", result);
    }
    
    // This would print an error message if uncommented
    // divide_safely(10, 0, &result);
    
    // Clean up
    free(array);
    
    return 0;
}