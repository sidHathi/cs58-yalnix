#ifndef _linked_list_h
#define _linked_list_h

#include <yalnix.h>
#include <hardware.h>

// Generic Linked List Structure
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#
// Modified to suit our needs

//data struct to hold the void* data but for the delayqueue version
typedef struct DelayNode {
    void *clock_ticks;
    void *process;
} DelayNode_t;

// A linked list (LL) node to store a queue entry

/*
* Note here we might want to switch to the pid being used depending on full funcitonality of this LL
* As of now I am unsure as to how one would obtain that ID without using somthing like the PID
* Its an easy fix though, we would just store the curr_process pid as the ID of the node itself.
*/
typedef struct lnode {
	int key; 				// Unique identifier
	struct lnode* next;  // Pointer to next node in the queue
  struct lnode* prev; // pointer to the previous node in the queue if it exists
	void* data;			// Pointer to data of current node
} lnode_t;

// Queue data structure
typedef struct linked_list {
	lnode_t *front, *rear;	// Pointers to front and back of queue
	int next_key;						// Key to assign to next node that gets enqueued
} linked_list_t;

//type def for a function passed as a parameter to free a queue for any data type
typedef void (*freeFunc)(void*);

// A utility function to create an empty queue
// Caller is responsible freeing queue memory later.
linked_list_t* linked_list_create();

// The function to add a new node to the queue
void linked_list_push(linked_list_t* ll, void* data);

// Function to pop data from the head of the queue
void* linked_list_remove(linked_list_t* ll, int key);

#endif /*!_linked_list_h*/
