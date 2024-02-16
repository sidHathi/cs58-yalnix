#include "queue.h"
#include <hardware.h>
#include <ylib.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#
// Modified to suit our needs

// Handle creation of new queue
// Caller is responsible for freeing this memory
queue_t*
queueCreate()
{
	queue_t* q = (queue_t*) malloc(sizeof(queue_t));
	q->front = q->rear = NULL;
	q->next_key = 0;
	return q;
}

// Enqueue new node
void
queuePush(queue_t* q, void* data)
{	
	// Allocate memory for new node
  helper_check_heap("eentering queue push");
	qnode_t* new_node = (qnode_t*) malloc(sizeof(qnode_t));
  helper_check_heap("queue node allocation");
	new_node->data = data;
	new_node->key = q->next_key;
	new_node->next = NULL;

	// If queue is empty, then new node is front and rear
	if (q->rear == NULL) {
		q->front = q->rear = new_node;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = new_node;
	q->rear = new_node;

	// Increment next_key
	q->next_key++;
  helper_check_heap("exiting queue push");
}

// Dequeue front node
void*
queuePop(queue_t* q)
{
	// If queue is empty, return NULL.
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

	// Return pointer to popped node's data
	return data;
}

int freeQueue(queue_t* q, freeFunc* freeFunction) {

  if (q == NULL) {
    return 1;
  };

  qnode_t* curr_node = q->front;
  /*
  *
  * typedef struct qnode {
	*   int key; 				// Unique identifier
	*   struct qnode* next;  // Pointer to next node in the queue
	*   void* data;			// Pointer to data of current node
  * } qnode_t;
  * Do we want to free the data, or have the function handle the deletion of the whole node?
  */
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

