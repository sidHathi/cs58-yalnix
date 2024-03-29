#ifndef _queue_h
#define _queue_h

#include <yalnix.h>
#include <hardware.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#
// Modified to suit our needs

// A linked list node to store a queue entry
typedef struct qnode {
	int key; 				// Unique identifier
	struct qnode* next;  // Pointer to next node in the queue
	void* data;			// Pointer to data of current node
} qnode_t;

// Queue data structure
typedef struct queue {
	qnode_t *front, *rear;	// Pointers to front and back of queue
	int next_key;						// Key to assign to next node that gets enqueued
	int count;
} queue_t;

//type def for a function passed as a parameter to free a queue for any data type
typedef void (*freeFunc)(void*);

// A utility function to create an empty queue
// Caller is responsible freeing queue memory later.
queue_t* queue_new();

// Push to back of queue. Returns 0 on success. Returns ERROR on fail.
int queue_push(queue_t* q, void* data);

// Pop from front of queue. Caller is responsible for freeing the void* pointer.
void* queue_pop(queue_t* q);

// Delete a queue. Calls itemdelete on data in each node, passing void* arg each time
int queue_delete(queue_t* q, void* arg, void (*itemdelete)(void* data, void* arg));

#endif /*!_queue_h*/
