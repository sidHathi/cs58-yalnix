#ifndef _lock_h
#define _lock_h

#include "queue.h"
#include <yalnix.h>
#include <hardware.h>

typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  Queue* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

#endif /*!_lock_h*/
