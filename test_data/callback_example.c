#include "callback_example.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 10
#define MAX_HANDLERS_PER_EVENT 5

// Structure to hold event handlers
typedef struct {
    char event_name[50];
    EventHandler handlers[MAX_HANDLERS_PER_EVENT];
    void* user_data[MAX_HANDLERS_PER_EVENT];
    int handler_count;
} EventRegistry;

// Global event registry
static EventRegistry events[MAX_EVENTS];
static int event_count = 0;

int process_ints(int* array, int length, IntProcessor processor) {
    int sum = 0;
    for (int i = 0; i < length; i++) {
        sum += processor(array[i]);
    }
    return sum;
}

void sort_array(void* array, int length, int element_size, Comparator compare) {
    qsort(array, length, element_size, compare);
}

void register_event_handler(const char* event_name, EventHandler handler, void* user_data) {
    // Find existing event or create new one
    int event_idx = -1;
    for (int i = 0; i < event_count; i++) {
        if (strcmp(events[i].event_name, event_name) == 0) {
            event_idx = i;
            break;
        }
    }
    
    if (event_idx == -1) {
        // Create new event
        if (event_count >= MAX_EVENTS) {
            printf("Error: Maximum number of events reached\n");
            return;
        }
        
        event_idx = event_count++;
        strncpy(events[event_idx].event_name, event_name, sizeof(events[event_idx].event_name) - 1);
        events[event_idx].handler_count = 0;
    }
    
    // Add handler to event
    if (events[event_idx].handler_count >= MAX_HANDLERS_PER_EVENT) {
        printf("Error: Maximum number of handlers for event '%s' reached\n", event_name);
        return;
    }
    
    int handler_idx = events[event_idx].handler_count++;
    events[event_idx].handlers[handler_idx] = handler;
    events[event_idx].user_data[handler_idx] = user_data;
    
    printf("Registered handler for event '%s'\n", event_name);
}

void trigger_event(const char* event_name) {
    // Find event
    for (int i = 0; i < event_count; i++) {
        if (strcmp(events[i].event_name, event_name) == 0) {
            printf("Triggering event '%s'\n", event_name);
            
            // Call all handlers
            for (int j = 0; j < events[i].handler_count; j++) {
                events[i].handlers[j](events[i].user_data[j], event_name);
            }
            
            return;
        }
    }
    
    printf("Warning: No handlers registered for event '%s'\n", event_name);
}

// Example callback implementations
int double_int(int value) {
    return value * 2;
}

int square_int(int value) {
    return value * value;
}

int compare_ints(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

// Example event handler
void log_event(void* user_data, const char* event_name) {
    const char* logger_name = (const char*)user_data;
    printf("[%s] Event '%s' occurred\n", logger_name, event_name);
}

// Test function for callbacks
void test_callbacks() {
    // Test processing integers
    int numbers[] = {1, 2, 3, 4, 5};
    int length = sizeof(numbers) / sizeof(numbers[0]);
    
    printf("Original array: ");
    for (int i = 0; i < length; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");
    
    // Process with double_int
    int double_sum = process_ints(numbers, length, double_int);
    printf("Sum after doubling each value: %d\n", double_sum);
    
    // Process with square_int
    int square_sum = process_ints(numbers, length, square_int);
    printf("Sum after squaring each value: %d\n", square_sum);
    
    // Test sorting
    int unsorted[] = {5, 2, 9, 1, 7};
    int unsorted_length = sizeof(unsorted) / sizeof(unsorted[0]);
    
    printf("Unsorted array: ");
    for (int i = 0; i < unsorted_length; i++) {
        printf("%d ", unsorted[i]);
    }
    printf("\n");
    
    sort_array(unsorted, unsorted_length, sizeof(int), compare_ints);
    
    printf("Sorted array: ");
    for (int i = 0; i < unsorted_length; i++) {
        printf("%d ", unsorted[i]);
    }
    printf("\n");
    
    // Test event system
    register_event_handler("app_start", log_event, "System Logger");
    register_event_handler("app_start", log_event, "Security Logger");
    register_event_handler("button_click", log_event, "UI Logger");
    
    trigger_event("app_start");
    trigger_event("button_click");
    trigger_event("nonexistent_event");
}

// Main function to test callbacks
int main() {
    test_callbacks();
    return 0;
}