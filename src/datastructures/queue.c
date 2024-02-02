#include "queue.h"
#include <hardware.h>
#include <ylib.h>

// Generic FIFO queue
// Source: https://www.geeksforgeeks.org/queue-linked-list-implementation/#

// A linked list (LL) node to store a queue entry
typedef struct qnode {
	int key;
	qnode_t* next;
} qnode_t;

// The queue, front stores the front node of LL and rear
// stores the last node of LL
typedef struct queue {
	qnode_t *front, *rear;
} queue_t;

qnode_t*
newNode(int k)
{
	qnode_t* temp
		= (qnode_t*)malloc(sizeof(qnode_t));
	temp->key = k;
	temp->next = NULL;
	return temp;
}

queue_t*
createQueue()
{
	queue_t* q
		= (queue_t*)malloc(sizeof(queue_t));
	q->front = q->rear = NULL;
	return q;
}

void
enQueue(queue_t* q, int k)
{
	// Create a new LL node
	qnode_t* temp = newNode(k);

	// If queue is empty, then new node is front and rear
	// both
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
}

void
deQueue(queue_t* q)
{
	// If queue is empty, return NULL.
	if (q->front == NULL)
		return;

	// Store previous front and move front one node ahead
	struct QNode* temp = q->front;

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;

	free(temp);
}