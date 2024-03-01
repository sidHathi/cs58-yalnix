#include "queue.h"
#include <hardware.h>
#include <ylib.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#
// Modified to suit our needs

// Create new queue
queue_t*
queue_new()
{
	queue_t* q = (queue_t*) malloc(sizeof(queue_t));
	if (q == NULL) {
		return NULL;
	}
	q->front = q->rear = NULL;
	q->next_key = 0;
	q->count = 0;
	return q;
}

// Enqueue new node
int
queue_push(queue_t* q, void* data)
{	
	if (q == NULL || data == NULL) {
		TracePrintf(1, "Queue Push: got null queue pointer or null data pointer\n");
		return ERROR;
	}

	qnode_t* new_node = (qnode_t*) malloc(sizeof(qnode_t));
	if (new_node == NULL) {
		TracePrintf(1, "Queue Push: failed to allocate memory for new queue node\n");
		return ERROR;
	}

	new_node->data = data;
	new_node->key = q->next_key;
	new_node->next = NULL;
	

	// If queue is empty, then new node is front and rear
	if (q->rear == NULL) {
		q->front = new_node;
		q->rear = new_node;
	}
	// Otherwise, push node to end
	else {
		q->rear->next = new_node;
		q->rear = new_node;
	}

	q->next_key++;
	q->count++;
  return 0;
}

// Dequeue front node
void*
queue_pop(queue_t* q)
{
	if (q == NULL) {
		return NULL;
	}

	if (q->front == NULL)
		return NULL;

	// Store previous front and move front one node ahead
	qnode_t* front_node = q->front;
	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;

	// Store pointer to popped node's data
	void* data = front_node->data;

	// Free the popped node
	free(front_node);

	// Decrement count
	q->count--;

	// Return pointer to popped node's data
	return data;
}

int queue_delete(queue_t* q, freeFunc* freeFunction) {
  if (q == NULL || freeFunction == NULL) {
		TracePrintf(1, "Queue Free: got null queue pointer or null free function pointer\n");
    return ERROR;
  };

  qnode_t* curr_node = q->front;

  while (curr_node != NULL) {
		qnode_t* next_node = curr_node->next;

		if(freeFunction != NULL) {
    	(*freeFunction)(curr_node->data);
		}
    free(curr_node);
    curr_node = next_node;
  }

  return 0;
};

