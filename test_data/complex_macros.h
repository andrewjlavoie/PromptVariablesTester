#ifndef COMPLEX_MACROS_H
#define COMPLEX_MACROS_H

#include <stdio.h>
#include <stdlib.h>

/* Basic macro definitions */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define SQUARE(a) ((a) * (a))

/* Error handling macros */
#define ERROR_RETURN(condition, retval, format, ...) \
    do { \
        if (condition) { \
            fprintf(stderr, format, ##__VA_ARGS__); \
            return retval; \
        } \
    } while(0)

#define CHECK_NULL(ptr, retval) \
    ERROR_RETURN((ptr) == NULL, retval, "Error: NULL pointer detected at %s:%d\n", __FILE__, __LINE__)

#define CHECK_ALLOC(ptr) \
    do { \
        if ((ptr) == NULL) { \
            fprintf(stderr, "Error: Memory allocation failed at %s:%d\n", __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

/* Loop macros */
#define FOR_EACH(item, array, length) \
    for (size_t keep = 1, i = 0, size = (length); keep && i < size; keep = !keep, i++) \
        for (item = (array)[i]; keep; keep = !keep)

/* Function-like macros */
#define SWAP(a, b, type) \
    do { \
        type temp = (a); \
        (a) = (b); \
        (b) = temp; \
    } while(0)

/* Data structure macros */
#define DECLARE_FLEXIBLE_ARRAY(type, name) \
    struct name { \
        size_t size; \
        type data[]; \
    };

#define INIT_FLEXIBLE_ARRAY(type, name, count) \
    (struct name*)malloc(sizeof(struct name) + sizeof(type) * (count))

/* Debugging macros */
#ifdef DEBUG
    #define DEBUG_PRINT(format, ...) fprintf(stderr, format, ##__VA_ARGS__)
    #define ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                fprintf(stderr, "Assertion failed: %s\nFile: %s, Line: %d\n", message, __FILE__, __LINE__); \
                abort(); \
            } \
        } while(0)
#else
    #define DEBUG_PRINT(format, ...)
    #define ASSERT(condition, message)
#endif

/* Define a dynamic array type */
DECLARE_FLEXIBLE_ARRAY(int, IntArray)

/* Example functions for using the macros */
void swap_example(int* a, int* b);
IntArray* create_int_array(size_t size);
void debug_example(void);

#endif /* COMPLEX_MACROS_H */