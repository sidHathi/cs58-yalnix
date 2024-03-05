#ifndef _pipe_h
#define _pipe_h

#include "queue.h"
#include "set.h"

typedef struct {
	int pipe_id;
	void* buffer;
	int num_bytes_available;
	queue_t* readers;
	set_t* writers;
} pipe_t;

#endif