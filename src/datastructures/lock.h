#ifndef _lock_h
#define _lock_h

#include "queue.h"
#include <yalnix.h>
#include <hardware.h>

typedef struct lock lock_t;

// creates a new lock with no owner
lock_t* lock_new(int lock_id);

#endif /*!_lock_h*/
