#ifndef COMPLEX_STRUCT_H
#define COMPLEX_STRUCT_H

#include "simple_struct.h"

/**
 * An enumeration for shapes
 */
enum ShapeType {
    CIRCLE = 0,
    RECTANGLE = 1,
    TRIANGLE = 2
};

/**
 * Base structure for all shapes
 */
struct Shape {
    enum ShapeType type;
    Point_t position;  /* Inheritance via composition */
    float area;
};

/**
 * Circle structure that "inherits" from Shape
 */
struct Circle {
    struct Shape base;  /* Inheritance via composition */
    float radius;
};

/**
 * Rectangle structure that "inherits" from Shape
 */
struct Rectangle {
    struct Shape base;  /* Inheritance via composition */
    float width;
    float height;
};

/**
 * Triangle structure that "inherits" from Shape
 */
struct Triangle {
    struct Shape base;  /* Inheritance via composition */
    Point_t vertices[3];  /* Uses Point from simple_struct.h */
};

/**
 * Initialize a circle
 */
struct Circle init_circle(Point_t center, float radius);

/**
 * Initialize a rectangle
 */
struct Rectangle init_rectangle(Point_t position, float width, float height);

/**
 * Initialize a triangle
 */
struct Triangle init_triangle(Point_t v1, Point_t v2, Point_t v3);

/**
 * Calculate the area of a shape
 */
float calculate_area(struct Shape* shape);

/**
 * Move a shape to a new position
 */
void move_shape(struct Shape* shape, Point_t new_position);

#endif /* COMPLEX_STRUCT_H */