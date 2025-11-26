#include <stdio.h>      // For printf()
#include <stdlib.h>     // For general utilities (not strictly needed here)
#include <pthread.h>    // For creating and managing threads
#include <unistd.h>     // For sleep/usleep() functions

// Define constants
#define QSIZE 4          // Queue size (number of items that can be held)
#define SHOPPER_ITEMS 5  // Number of items the shopper will add

// Global variables shared between threads
int queue[QSIZE];        // Circular queue (array to hold items)
int front = 0;           // Index of the front of the queue
int rear = 0;            // Index where next item will be inserted
int count = 0;           // Number of items currently in the queue

// Shopper thread function — adds items to the queue
void *shopper(void *arg) {
    int i;
    for (i = 0; i < SHOPPER_ITEMS; i++) {      // Loop for each shopper item
        int item = 100 + i;                    // Generate an item ID (100, 101, ...)

        // Check if the queue is not full
        if (count < QSIZE) {
            queue[rear] = item;                // Add the item to the queue
            rear = (rear + 1) % QSIZE;         // Move rear index forward (wrap around using modulo)
            count++;                           // Increase queue count
            printf("Shopper 1: Added item %d (queue size: %d, front=%d, rear=%d)\n",
                   item, count, front, rear);  // Print queue status
        } else {
            // If queue is full, the shopper can’t add an item
            printf("Shopper 1: Queue full, cannot add item %d\n", item);
        }

        usleep(300000);  // Simulate delay between adding items (0.3 seconds)
    }

    // Indicate that the shopper has finished adding all items
    printf("Shopper 1: finished adding items\n");

    pthread_exit(NULL);  // Exit the shopper thread
}

// Cashier thread function — processes items from the queue
void *cashier(void *arg) {
    int total = 0;  // Total number of items processed

    while (1) {     // Run indefinitely until all items processed
        if (count > 0) {                          // Check if queue has items
            int item = queue[front];              // Get the next item from the queue
            front = (front + 1) % QSIZE;          // Move front index forward (circularly)
            count--;                              // Decrease queue count
            total++;                              // Increase number of processed items
            printf("Cashier 1: Processed item %d (total: %d, queue size: %d, front=%d, rear=%d)\n",
                   item, total, count, front, rear);  // Print processing info
        } else {
            // Queue is empty — cashier must wait for items
            printf("Cashier 1: Queue empty, waiting...\n");
            usleep(500000);                       // Sleep for 0.5 seconds before checking again
        }

        // Stop once all shopper items are processed
        if (total == SHOPPER_ITEMS) {
            printf("Cashier 1: finished processing (total %d)\n", total);
            break;  // Exit the loop
        }
    }

    pthread_exit(NULL);  // Exit the cashier thread
}

int main() {
    // Display header info
    printf("=== Phase 1: Single-lane checkout (NO SYNCHRONIZATION) ===\n");
    printf("QSIZE: %d\tSHOPPER_ITEMS: %d\n\n", QSIZE, SHOPPER_ITEMS);

    // Declare thread handles
    pthread_t shopper_thread, cashier_thread;

    // Create two threads: one for shopper and one for cashier
    pthread_create(&shopper_thread, NULL, shopper, NULL);
    pthread_create(&cashier_thread, NULL, cashier, NULL);

    // Wait for both threads to finish before exiting
    pthread_join(shopper_thread, NULL);
    pthread_join(cashier_thread, NULL);

    // Indicate that the simulation is done
    printf("\n=== Phase 1 Complete ===\n");

    return 0;  // End of program
}