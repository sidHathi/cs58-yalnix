#ifndef _cvar_h
#define _cvar_h

#include "queue.h"

typedef struct cvar {
  int cvar_id; // unique identifier
  queue_t* blocked; // FIFO queue of PCB's waiting on cvar
} cvar_t;

#endif