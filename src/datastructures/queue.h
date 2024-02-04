#ifndef _queue_h
#define _queue_h

#include <yalnix.h>
#include <hardware.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#

// A linked list (LL) node to store a queue entry
typedef struct qnode qnode_t;

// The queue, front stores the front node of LL and rear
// stores the last node of LL
typedef struct queue queue_t;

// A utility function to create a new linked list node.
qnode_t* newNode(int k);

// A utility function to create an empty queue
// Caller is responsible freeing Queue later.
// Remember to free entries in Queue before freeing the Queue itself.
queue_t* createQueue();

// The function to add a key k to q
void enQueue(queue_t* q, int k);

// Function to remove a key from given queue q
int deQueue(queue_t* q);

#endif /*!_queue_h*/
