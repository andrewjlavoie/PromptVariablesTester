#include "union_example.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Value create_int_value(int i) {
    Value value;
    value.type = TYPE_INT;
    value.data.i = i;
    return value;
}

Value create_float_value(float f) {
    Value value;
    value.type = TYPE_FLOAT;
    value.data.f = f;
    return value;
}

Value create_string_value(const char* s) {
    Value value;
    value.type = TYPE_STRING;
    // Allocate memory for the string
    value.data.s = strdup(s);
    return value;
}

Value create_point_value(int x, int y) {
    Value value;
    value.type = TYPE_POINT;
    value.data.p.x = x;
    value.data.p.y = y;
    return value;
}

void print_value(const Value* value) {
    if (!value) {
        printf("NULL value\n");
        return;
    }
    
    switch (value->type) {
        case TYPE_INT:
            printf("Int: %d\n", value->data.i);
            break;
        case TYPE_FLOAT:
            printf("Float: %f\n", value->data.f);
            break;
        case TYPE_STRING:
            printf("String: %s\n", value->data.s ? value->data.s : "NULL");
            break;
        case TYPE_POINT:
            printf("Point: (%d, %d)\n", value->data.p.x, value->data.p.y);
            break;
        default:
            printf("Unknown type\n");
            break;
    }
}