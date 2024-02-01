#ifndef _datastructures_h
#define _datastructures_h
#define QUEUE_SIZE 100 // Maximum number of elements in FIFO queue

#include <hardware.h>
#include <ylib.h>

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

// Lock for synchronization
typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  Queue* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

// Cvar for synchronization
typedef struct cvar {
  int cvar_id; // unique identifier
  int owner; // pid of process using cvar
  Queue* blocked; // FIFO queue of PCB's waiting on cvar
} cvar_t;

#endif /*_datastructures_h*/