#ifndef CALLBACK_EXAMPLE_H
#define CALLBACK_EXAMPLE_H

/**
 * Callback function type for processing integers
 */
typedef int (*IntProcessor)(int value);

/**
 * Callback function type for comparing two values
 */
typedef int (*Comparator)(const void* a, const void* b);

/**
 * Callback function type that takes user data
 */
typedef void (*EventHandler)(void* user_data, const char* event_name);

/**
 * Process an array of integers using the provided callback
 * Returns the sum of the results from the callback
 */
int process_ints(int* array, int length, IntProcessor processor);

/**
 * Sort an array using the provided comparator
 */
void sort_array(void* array, int length, int element_size, Comparator compare);

/**
 * Register an event handler
 */
void register_event_handler(const char* event_name, EventHandler handler, void* user_data);

/**
 * Trigger an event
 */
void trigger_event(const char* event_name);

/**
 * Example callbacks
 */
int double_int(int value);
int square_int(int value);
int compare_ints(const void* a, const void* b);

#endif /* CALLBACK_EXAMPLE_H */