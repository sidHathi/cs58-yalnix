#ifndef _pipe_h
#define _pipe_h

#include <yalnix.h>
#include <hardware.h>
#include "queue.h"
#include "kernel.h"
#include "pcb.h"
#include "util.h"

typedef struct pipe pipe_t;

pipe_t* pipe_new(int id);

int pipe_write(pipe_t* pipe, void* data, int len);

int pipe_read(pipe_t* pipe, void* data, int len);

void pipe_free(pipe_t* pipe);

#endif /*!_pipe_h*/
