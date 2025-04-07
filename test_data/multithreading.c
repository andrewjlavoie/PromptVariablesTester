#include "multithreading.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void queue_init(ThreadSafeQueue* queue, int capacity) {
    queue->data = (int*)malloc(sizeof(int) * capacity);
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
}

void queue_destroy(ThreadSafeQueue* queue) {
    free(queue->data);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
}

void queue_enqueue(ThreadSafeQueue* queue, int item) {
    pthread_mutex_lock(&queue->mutex);
    
    // Wait until the queue is not full
    while (queue->size == queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    // Add the item to the queue
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->data[queue->rear] = item;
    queue->size++;
    
    // Signal that the queue is not empty
    pthread_cond_signal(&queue->not_empty);
    
    pthread_mutex_unlock(&queue->mutex);
}

int queue_dequeue(ThreadSafeQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    
    // Wait until the queue is not empty
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    // Remove an item from the queue
    int item = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    
    // Signal that the queue is not full
    pthread_cond_signal(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
    
    return item;
}

void* producer_function(void* arg) {
    WorkerArgs* args = (WorkerArgs*)arg;
    int id = args->id;
    ThreadSafeQueue* queue = args->queue;
    int num_items = args->num_items_to_process;
    
    printf("Producer %d started\n", id);
    
    for (int i = 0; i < num_items; i++) {
        int item = id * 1000 + i;
        queue_enqueue(queue, item);
        printf("Producer %d produced %d\n", id, item);
        
        // Sleep for a random time to simulate work
        usleep((rand() % 500 + 100) * 1000);
    }
    
    printf("Producer %d finished\n", id);
    return NULL;
}

void* consumer_function(void* arg) {
    WorkerArgs* args = (WorkerArgs*)arg;
    int id = args->id;
    ThreadSafeQueue* queue = args->queue;
    int num_items = args->num_items_to_process;
    
    printf("Consumer %d started\n", id);
    
    for (int i = 0; i < num_items; i++) {
        int item = queue_dequeue(queue);
        printf("Consumer %d consumed %d\n", id, item);
        
        // Sleep for a random time to simulate work
        usleep((rand() % 500 + 100) * 1000);
    }
    
    printf("Consumer %d finished\n", id);
    return NULL;
}

void run_multithreading_demo(int num_producers, int num_consumers) {
    // Seed the random number generator
    srand(time(NULL));
    
    // Initialize the queue
    ThreadSafeQueue queue;
    queue_init(&queue, 10);
    
    // Create thread arrays
    pthread_t* producer_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_producers);
    pthread_t* consumer_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_consumers);
    
    // Create thread arguments
    WorkerArgs* producer_args = (WorkerArgs*)malloc(sizeof(WorkerArgs) * num_producers);
    WorkerArgs* consumer_args = (WorkerArgs*)malloc(sizeof(WorkerArgs) * num_consumers);
    
    // Calculate the total number of items to produce/consume
    int total_items = 20;
    int items_per_producer = total_items / num_producers;
    int items_per_consumer = total_items / num_consumers;
    
    printf("Starting multithreading demo with %d producers and %d consumers\n", 
           num_producers, num_consumers);
    printf("Each producer will produce %d items, each consumer will consume %d items\n",
           items_per_producer, items_per_consumer);
    
    // Create and start the producer threads
    for (int i = 0; i < num_producers; i++) {
        producer_args[i].id = i;
        producer_args[i].queue = &queue;
        producer_args[i].num_items_to_process = items_per_producer;
        
        pthread_create(&producer_threads[i], NULL, producer_function, &producer_args[i]);
    }
    
    // Create and start the consumer threads
    for (int i = 0; i < num_consumers; i++) {
        consumer_args[i].id = i;
        consumer_args[i].queue = &queue;
        consumer_args[i].num_items_to_process = items_per_consumer;
        
        pthread_create(&consumer_threads[i], NULL, consumer_function, &consumer_args[i]);
    }
    
    // Wait for all producers to finish
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producer_threads[i], NULL);
    }
    
    // Wait for all consumers to finish
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumer_threads[i], NULL);
    }
    
    printf("All threads have completed\n");
    
    // Clean up
    queue_destroy(&queue);
    free(producer_threads);
    free(consumer_threads);
    free(producer_args);
    free(consumer_args);
}

int main() {
    run_multithreading_demo(2, 3);
    return 0;
}