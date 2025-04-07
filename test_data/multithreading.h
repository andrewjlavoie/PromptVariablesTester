#ifndef MULTITHREADING_H
#define MULTITHREADING_H

#include <pthread.h>

/* Thread-safe queue structure */
typedef struct {
    int* data;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ThreadSafeQueue;

/* Worker thread function argument */
typedef struct {
    int id;
    ThreadSafeQueue* queue;
    int num_items_to_process;
} WorkerArgs;

/* Initialize a thread-safe queue */
void queue_init(ThreadSafeQueue* queue, int capacity);

/* Destroy a thread-safe queue */
void queue_destroy(ThreadSafeQueue* queue);

/* Enqueue an item (thread-safe) */
void queue_enqueue(ThreadSafeQueue* queue, int item);

/* Dequeue an item (thread-safe) */
int queue_dequeue(ThreadSafeQueue* queue);

/* Producer thread function */
void* producer_function(void* arg);

/* Consumer thread function */
void* consumer_function(void* arg);

/* Run a multithreading demo with producers and consumers */
void run_multithreading_demo(int num_producers, int num_consumers);

#endif /* MULTITHREADING_H */