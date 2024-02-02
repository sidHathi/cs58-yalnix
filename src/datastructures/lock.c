#include <lock.h>
#include <queue.h>

typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  queue_t* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

lock_t*
lock_new(int lock_id)
{
  // allocate memory for new lock
}

void
lock_free(lock_t* lock)
{
  // free lock
}
