#include "complex_struct.h"
#include <math.h>

struct Circle init_circle(Point_t center, float radius) {
    struct Circle circle;
    circle.base.type = CIRCLE;
    circle.base.position = center;
    circle.radius = radius;
    circle.base.area = M_PI * radius * radius;
    return circle;
}

struct Rectangle init_rectangle(Point_t position, float width, float height) {
    struct Rectangle rect;
    rect.base.type = RECTANGLE;
    rect.base.position = position;
    rect.width = width;
    rect.height = height;
    rect.base.area = width * height;
    return rect;
}

struct Triangle init_triangle(Point_t v1, Point_t v2, Point_t v3) {
    struct Triangle triangle;
    triangle.base.type = TRIANGLE;
    
    // Set the center of the triangle as its position (average of vertices)
    triangle.base.position.x = (v1.x + v2.x + v3.x) / 3;
    triangle.base.position.y = (v1.y + v2.y + v3.y) / 3;
    
    // Store vertices
    triangle.vertices[0] = v1;
    triangle.vertices[1] = v2;
    triangle.vertices[2] = v3;
    
    // Calculate area using Heron's formula
    float a = distance(v1, v2);
    float b = distance(v2, v3);
    float c = distance(v3, v1);
    float s = (a + b + c) / 2; // semi-perimeter
    triangle.base.area = sqrt(s * (s - a) * (s - b) * (s - c));
    
    return triangle;
}

float calculate_area(struct Shape* shape) {
    return shape->area;
}

void move_shape(struct Shape* shape, Point_t new_position) {
    shape->position = new_position;
}