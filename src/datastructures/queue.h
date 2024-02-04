#ifndef _queue_h
#define _queue_h

#include <yalnix.h>
#include <hardware.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#
// Modified to suit our needs

// A linked list (LL) node to store a queue entry
typedef struct qnode qnode_t;

// Queue data structure
typedef struct queue queue_t;

// A utility function to create an empty queue
// Caller is responsible freeing queue memory later.
queue_t* queueCreate();

// The function to add a new node to the queue
void queuePush(queue_t* q, void* data);

// Function to pop data from the head of the queue
void* queuePop(queue_t* q);

// IMPORTANT: NEED FREE FUNCTION

#endif /*!_queue_h*/
