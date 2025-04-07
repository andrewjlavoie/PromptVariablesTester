#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/**
 * A generic node in a linked list
 */
typedef struct Node {
    void* data;
    struct Node* next;
} Node;

/**
 * A linked list structure
 */
typedef struct LinkedList {
    Node* head;
    Node* tail;
    int size;
} LinkedList;

/**
 * Initialize a new empty linked list
 */
LinkedList* list_init();

/**
 * Free all memory used by the linked list
 */
void list_free(LinkedList* list);

/**
 * Add an item to the end of the list
 */
void list_append(LinkedList* list, void* data);

/**
 * Insert an item at the beginning of the list
 */
void list_prepend(LinkedList* list, void* data);

/**
 * Remove an item from the list
 * Returns 1 if item was found and removed, 0 otherwise
 */
int list_remove(LinkedList* list, void* data, int (*compare)(void*, void*));

/**
 * Find an item in the list
 * Returns the node containing the item, or NULL if not found
 */
Node* list_find(LinkedList* list, void* data, int (*compare)(void*, void*));

/**
 * Get the number of items in the list
 */
int list_size(LinkedList* list);

#endif /* LINKED_LIST_H */