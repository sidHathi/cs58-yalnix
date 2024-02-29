#include "lock.h"
#include "kernel.h"




/*
* typedef struct lock {
*   int lock_id; // unique identifier
*   int owner; // pid of process controlling lock, -1 if unlocked
*   queue_t* blocked; // FIFO queue of PCB's waiting for lock
* } lock_t;
*/
int lock_init(int *lock_idp) {
  TracePrintf(1, "Lock_Init: Initializing Lock...\n");

  lock_t* new_lock = (lock_t*) malloc(sizeof(lock_t));
  if (new_lock == NULL) {
    return NULL;
  }

  new_lock->lock_id = next_lock_id;
  next_lock_id ++;

  new_lock->owner = NULL;

  queue_t* lock_queue = queueCreate();

  if (lock_queue == NULL) {
    TracePrintf(1, "Lock Init: Failed to create blocked process queue for lock!\n");
    return ERROR;
  }

  // need to figure this part out;
  lock_idp = new_lock;

  new_lock->blocked = lock_queue;

  if (set_insert(locks, new_lock->lock_id, new_lock) != 0) {
    TracePrintf(1, "Lock Init: Failed to add new lock into locks set!\n");
    return ERROR;
  };

  return 0;

}

int aquire_lock(int lock_id) {
  lock_t* curr_lock = set_find(locks, lock_id);

  if (curr_lock == NULL) {
    TracePrintf(1, "Aquire Lock: Error while aqquiring lock, check to make sure ID is valid\n");
    return ERROR;
  }

  //check if the current process already own the lock for safety

  if (curr_lock->owner = current_process->pid) {
    TracePrintf(1, "Aquire Lock: Cannot aquire lock as it is already owned by this process\n");
    return ERROR;
  }

  //if its owned by someone else currently, then add it to the queue, and let the scheduler take over.
  if (curr_lock->owner != NULL) {
    // may need to change this back to the process itself not just the pid
    if(queuePush(curr_lock->blocked, current_process->pid) != 0) {
      TracePrintf(1, "Aquire Lock: Failed to add process to blocked queue\n");
      return ERROR;
    }

    // scheduler();
  }
  
  curr_lock->owner = current_process->pid;


  return 0;
}

int release_lock(int lock_id) {

  
}


//have to check the reality of this 
void lock_free(lock_t* lock_idp) {
  // free lock
}
