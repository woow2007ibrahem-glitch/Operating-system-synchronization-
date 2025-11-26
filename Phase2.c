#include <stdio.h>      // For printf() Scanf()
#include <stdlib.h>     // For malloc and pointer
#include <pthread.h>    // For creating and managing threads
#include <unistd.h>     // For Unix function like fork() sleep() pid()
#include <semaphore.h>  // For define the semaphore object

// Define constants
#define QSIZE 4          // Queue size (number of items that can be hold)
#define SHOPPER_ITEMS 4  // Number of items each shopper will add
#define NUM_SHOPPERS 2   // Number of shopper threads

// Global variables shared between threads
int queue[QSIZE];        // Circular queue (array to hold items) fixed array
int front = 0;           // Index of the front of the queue 
int rear = 0;            // Index where next item will be inserted
int count = 0;           // Number of items currently in the queue

pthread_mutex_t mutex;   // mutex tool
sem_t empty;             // semaphore for empty slots
sem_t full;              // semaphore for full slots

// Shopper thread function — adds items to the queue (Producer)
void* shopper(void *arg) { 
    int id = (int)(long)arg;     // Shopper ID (1 or 2)

    for (int i = 0; i < SHOPPER_ITEMS; i++) {    
        int item = id * 100 + i;     // Example: shopper 1 = 100,101… shopper 2 = 200,201…

        sem_wait(&empty);            // Wait until queue has empty slot
        pthread_mutex_lock(&mutex);  // Lock queue for safe access

        queue[rear] = item;          // Add the item to the queue
        rear = (rear + 1) % QSIZE;   // Move rear index forward (wrap around)
        count++;                     // Increase queue count

        printf("Shopper %d: Added item %d (queue size: %d, front=%d, rear=%d)\n",
               id, item, count, front, rear);  

        pthread_mutex_unlock(&mutex); // Release queue
        sem_post(&full);             // Signal that queue has a full slot

        usleep(300000);              // Simulate delay (0.3 seconds)
    }

    printf("Shopper %d: finished adding items\n", id);
    pthread_exit(NULL);  // Exit the shopper thread
}

// Cashier thread function — processes items from the queue (Consumer)
void* cashier(void *arg) {
    int total = 0;                         // Total items processed
    int expected = NUM_SHOPPERS * SHOPPER_ITEMS;

    while (total < expected) {             // Continue until all items processed
        sem_wait(&full);                   // Wait until queue has an item
        pthread_mutex_lock(&mutex);        // Safe access

        int item = queue[front];           // Take item from queue
        front = (front + 1) % QSIZE;       // Move front index forward
        count--;                           // Decrease queue size
        total++;                           // Count processed item

        printf("Cashier: Processed item %d (total: %d, queue size: %d, front=%d, rear=%d)\n",
               item, total, count, front, rear);

        pthread_mutex_unlock(&mutex);      // Release queue
        sem_post(&empty);                  // Signal empty slot

        usleep(500000);                    // Delay (0.5 seconds)
    }

    printf("Cashier: finished processing (total %d)\n", total);
    pthread_exit(NULL);
}

int main() {
    // Display header info
    printf("=== Phase 2: Single-lane checkout (SYNCHRONIZED) ===\n");
    printf("QSIZE: %d\tSHOPPER_ITEMS: %d\tNUM_SHOPPERS: %d\n\n",
            QSIZE, SHOPPER_ITEMS, NUM_SHOPPERS);

    // Initialize mutex + semaphores
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, QSIZE); // QSIZE empty slots initially
    sem_init(&full, 0, 0);      // 0 full slots initially

    pthread_t shoppers[NUM_SHOPPERS];
    pthread_t cashier_thread;

    // Create shopper threads
    for (int i = 0; i < NUM_SHOPPERS; i++)
        pthread_create(&shoppers[i], NULL, shopper, (void*)(long)(i + 1));

    // Create cashier thread
    pthread_create(&cashier_thread, NULL, cashier, NULL);

    // Wait for shoppers to finish
    for (int i = 0; i < NUM_SHOPPERS; i++)
        pthread_join(shoppers[i], NULL);

    // Wait for cashier
    pthread_join(cashier_thread, NULL);

    // Clean up
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("\n=== Phase 2 Complete (synchronized) ===\n");
    return 0;
}
