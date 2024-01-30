#ifndef _queue_h
#define _queue_h

#include <hardware.h>
#include <ylib.h>

typedef struct queue queue_t;

queue_t* queue_new();

queue_t* queue_push(queue_t* queue, void* elem);

void* queue_pop(queue_t* queue);

void queue_delete(queue_t* queue);

#endif /*!_queue_h*/