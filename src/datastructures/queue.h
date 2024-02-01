#ifndef _queue_h
#define _queue_h

#include <yalnix.h>
#include <hardware.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#

// A linked list (LL) node to store a queue entry
struct QNode {
	int key;
	struct QNode* next;
};

// The queue, front stores the front node of LL and rear
// stores the last node of LL
struct Queue {
	struct QNode *front, *rear;
};

// A utility function to create a new linked list node.
struct QNode* newNode(int k);

// A utility function to create an empty queue
// Caller is responsible freeing Queue later.
// Remember to free entries in Queue before freeing the Queue itself.
struct Queue* createQueue();

// The function to add a key k to q
void enQueue(struct Queue* q, int k);

// Function to remove a key from given queue q
void deQueue(struct Queue* q);

#endif /*!_queue_h*/
