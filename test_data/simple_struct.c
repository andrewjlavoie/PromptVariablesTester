#include "simple_struct.h"
#include <math.h>

Point_t init_point(int x, int y) {
    Point_t p;
    p.x = x;
    p.y = y;
    return p;
}

float distance(Point_t p1, Point_t p2) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    return sqrt(dx*dx + dy*dy);
}