#ifndef UNION_EXAMPLE_H
#define UNION_EXAMPLE_H

#include <stdint.h>

/**
 * Different types of values that can be stored
 */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_POINT
} ValueType;

/**
 * Point structure for spatial data
 */
typedef struct {
    int x;
    int y;
} Point;

/**
 * Union for storing different types of data
 */
typedef union {
    int i;
    float f;
    char* s;
    Point p;
} ValueData;

/**
 * A generic value container that can hold different types
 */
typedef struct {
    ValueType type;
    ValueData data;
} Value;

/**
 * Create a new integer value
 */
Value create_int_value(int i);

/**
 * Create a new float value
 */
Value create_float_value(float f);

/**
 * Create a new string value
 */
Value create_string_value(const char* s);

/**
 * Create a new point value
 */
Value create_point_value(int x, int y);

/**
 * Print a value
 */
void print_value(const Value* value);

#endif /* UNION_EXAMPLE_H */