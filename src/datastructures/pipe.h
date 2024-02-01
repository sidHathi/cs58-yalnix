#ifndef _pcb_h
#define _pcb_h

#include <yalnix.h>
#include <hardware.h>
#include "queue.h"

typedef struct pipe_LL {
  pipe_t* head;
} pipe_LL_t;

typedef struct pipe {
  int id;
  Queue* readers;
  Queue* writers;
  pipe_t* next; // next p
  char* data;
} pipe_t;

#endif /*!_pcb_h*/
