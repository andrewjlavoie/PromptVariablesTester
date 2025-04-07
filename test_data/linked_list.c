#include "linked_list.h"
#include <stdlib.h>

LinkedList* list_init() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list) {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
    return list;
}

void list_free(LinkedList* list) {
    if (!list) return;
    
    Node* current = list->head;
    while (current) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    
    free(list);
}

void list_append(LinkedList* list, void* data) {
    if (!list) return;
    
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return;
    
    node->data = data;
    node->next = NULL;
    
    if (!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    
    list->size++;
}

void list_prepend(LinkedList* list, void* data) {
    if (!list) return;
    
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return;
    
    node->data = data;
    node->next = list->head;
    
    if (!list->head) {
        list->tail = node;
    }
    
    list->head = node;
    list->size++;
}

int list_remove(LinkedList* list, void* data, int (*compare)(void*, void*)) {
    if (!list || !list->head) return 0;
    
    Node* current = list->head;
    Node* prev = NULL;
    
    while (current) {
        if (compare(current->data, data)) {
            if (prev) {
                prev->next = current->next;
                if (current == list->tail) {
                    list->tail = prev;
                }
            } else {
                list->head = current->next;
                if (current == list->tail) {
                    list->tail = NULL;
                }
            }
            
            free(current);
            list->size--;
            return 1;
        }
        
        prev = current;
        current = current->next;
    }
    
    return 0;
}

Node* list_find(LinkedList* list, void* data, int (*compare)(void*, void*)) {
    if (!list) return NULL;
    
    Node* current = list->head;
    while (current) {
        if (compare(current->data, data)) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int list_size(LinkedList* list) {
    return list ? list->size : 0;
}