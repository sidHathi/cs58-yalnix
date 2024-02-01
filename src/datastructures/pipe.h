#ifndef _pcb_h
#define _pcb_h

#include <yalnix.h>
#include <hardware.h>
#include "queue.h"

typedef struct pipe_LL pipe_LL_t;

typedef struct pipe pipe_t;

pipe_t* pipe_new(int id);

pipe_LL_t* pipe_LL_new(pipe_t* head);

void pipe_free(pipe_t* pipe);

void pipe_LL_free(pipe_LL_t* pipe_LL);

#endif /*!_pcb_h*/
