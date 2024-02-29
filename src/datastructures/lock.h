#ifndef _lock_h
#define _lock_h

#include "queue.h"
#include <yalnix.h>
#include <hardware.h>

typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  queue_t* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

// creates a new lock with no owner
int lock_init(int *lock_idp);

int aquire_lock(int lock_id);

int release_lock(int lock_id);

#endif /*!_lock_h*/
