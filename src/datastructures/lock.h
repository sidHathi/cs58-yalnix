#ifndef _lock_h
#define _lock_h

#include "queue.h"

typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  queue_t* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

#endif