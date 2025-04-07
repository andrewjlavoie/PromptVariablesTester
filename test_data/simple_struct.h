#ifndef SIMPLE_STRUCT_H
#define SIMPLE_STRUCT_H

/**
 * A simple structure that represents a point in 2D space
 */
struct Point {
    int x;
    int y;
};

/**
 * A typedef for the Point struct
 */
typedef struct Point Point_t;

/**
 * Initialize a point with x and y coordinates
 */
Point_t init_point(int x, int y);

/**
 * Calculate the distance between two points
 */
float distance(Point_t p1, Point_t p2);

#endif /* SIMPLE_STRUCT_H */