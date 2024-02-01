#ifndef _cvar_h
#define _cvar_h

#include <yalnix.h>
#include <hardware.h>
#include "queue.h"

typedef struct cvar {
  int cvar_id; // unique identifier
  int owner; // pid of process using cvar
  Queue* blocked; // FIFO queue of PCB's waiting on cvar
} cvar_t;

#endif /*!_cvar_h*/
