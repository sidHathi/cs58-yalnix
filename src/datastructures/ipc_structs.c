#include "ipc_structs.h"
#include "kernel.h"
#include "pcb.h"

// IPC helper function to find the proper set
set_t* ipc_get_set(ipc_wrapper_t* ipc_wrapper, int ipc_type) {
  if (ipc_wrapper == NULL) {
    return NULL;
  }

  if (ipc_type == LOCK) {
    return ipc_wrapper->locks;
  }

  else if (ipc_type == CVAR) {
    return ipc_wrapper->cvars;
  }

  else if (ipc_wrapper == PIPE) {
    return ipc_wrapper->pipes;
  }

  else {
    TracePrintf(1, "No such ipc_type %d\n", ipc_type);
    return NULL;
  }
}

// void ipc_free(ipc_wrapper_t* ipc_wrapper, int ipc_type, int ipc_id) {
//   if (ipc_wrapper == NULL) {
//     return;
//   }


// }

// create a new IPC Wrapper
ipc_wrapper_t* ipc_wrapper_init() {
  ipc_wrapper_t* ipc_wrapper = (ipc_wrapper_t*) malloc(sizeof(ipc_wrapper_t));
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "IPC Wrapper Init: Couldn't malloc\n");
    return NULL;
  }

  ipc_wrapper->next_ipc_id = 0;
  

  ipc_wrapper->locks = set_new();
  if (ipc_wrapper->locks == NULL) {
    free(ipc_wrapper);
    return NULL;
  }

  ipc_wrapper->cvars = set_new();
  if (ipc_wrapper->cvars == NULL) {
    free(ipc_wrapper->locks);
    free(ipc_wrapper);
    return NULL;
  }

  ipc_wrapper->pipes = set_new();
  if (ipc_wrapper->pipes == NULL) {
    free(ipc_wrapper->locks);
    free(ipc_wrapper->cvars);
    free(ipc_wrapper);
    return NULL;
  }
}

//delete an existing IPC Wrapper
void ipc_wrapper_delete(ipc_wrapper_t* ipc_wrapper) {
  if(ipc_wrapper == NULL) {
    return;
  }

  if (ipc_wrapper->locks != NULL) {
    free(ipc_wrapper->locks);
  }

  if (ipc_wrapper->cvars != NULL) {
    free(ipc_wrapper->cvars);
  }

  if (ipc_wrapper->pipes != NULL) {
    free(ipc_wrapper->pipes);
  }

  free(ipc_wrapper);
}

// PARAMS: takes in the current ipc wrapper, the types as defined above, and the id of either the lock, cvar, or pipe
// int ipc_reclaim(ipc_wrapper_t* ipc_wrapper, int ipc_type, int ipc_id) {
//   if (ipc_wrapper == NULL) {
//     TracePrintf(1, "IPC Wrapper: cannot reclaim on null ipc_wrapper\n");
//     return ERROR;
//   }

//   set_t* ipc_set = ipc_get_set(ipc_wrapper, ipc_type);
//   if (ipc_set == NULL) {
//     return NULL;
//   } 

//   set_pop(ipc_set, ipc_id);
// }

/******** LOCK FUNCTIONALITY ********/

int lock_init(ipc_wrapper_t* ipc_wrapper) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Lock Init: IPC Wrapper is NULL\n");
    return ERROR;
  }

  lock_t* new_lock = (lock_t*) malloc(sizeof(lock_t));

  if (new_lock == NULL) {
    TracePrintf(1, "Lock Init: Couldn't malloc new lock\n");
    return ERROR;
  }

  new_lock->lock_id = ipc_wrapper->next_ipc_id;
  ipc_wrapper->next_ipc_id++;

  new_lock->owner = UNLOCKED;

  queue_t* lock_queue = queueCreate();

  if (lock_queue == NULL) {
    TracePrintf(1, "Lock Init: Failed to create blocked process queue for lock!\n");
    free(new_lock);
    return ERROR;
  }

  new_lock->blocked = lock_queue;

  return new_lock->lock_id;

}


int lock_acquire(ipc_wrapper_t* ipc_wrapper, int lock_id) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Lock Acquire: IPC Wrapper is NULL\n");
    return ERROR;
  }

  set_t* lock_set = ipc_get_set(ipc_wrapper, LOCK);

  if (lock_set == NULL) {
    TracePrintf(1, "Lock Acquire: Lock set is NULL\n");
    return ERROR;
  }

  lock_t* lock = set_find(lock_set, lock_id);

  if (lock == NULL) {
    TracePrintf(1, "Lock Acquire: Popped lock is NULL\n");
    return ERROR;
  }

  if (lock->owner == UNLOCKED) {
    lock->owner = current_process->pid;
    TracePrintf(1, "Lock Acquire: Current process %d has acquired the lock\n", current_process->pid);
    return ACQUIRE_SUCCESS;
  }
  else if (lock->owner == current_process->pid) {
    // design choice, we are going to throw an error if a process tries to claim a lock they already own
    TracePrintf(1, "Lock Acquire: Failed to acquire lock, current process already owns lock\n");
    return ERROR;
  }
  else {
    if(queuePush(lock->blocked, current_process) != 0) {
      TracePrintf(1, "Lock Acquire: Failed to add process to blocked queue\n");
      return ERROR;
    }
    TracePrintf(1, "Lock Acquire: Current process %d added to queue, waiting to acquire lock\n", current_process->pid);
    return ACQUIRE_BLOCKED;
  }
}

//function to release the lock
int lock_release(ipc_wrapper_t* ipc_wrapper, int lock_id) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Lock Release: IPC Wrapper is NULL\n");
    return ERROR;
  }

  set_t* lock_set = ipc_get_set(ipc_wrapper, LOCK);

  if (lock_set == NULL) {
    TracePrintf(1, "Lock Release: Lock set is NULL\n");
    return ERROR;
  }

  lock_t* lock = set_find(lock_set, lock_id);

  if (lock == NULL) {
    TracePrintf(1, "Lock Release: Popped lock is NULL\n");
    return ERROR;
  }

  if (current_process->pid != lock->owner) {
    TracePrintf(1, "Lock Release: failed to release because process %d does not own lock %d\n", current_process->pid, lock->owner);
    return ERROR;
  }

  if (lock->blocked == NULL) {
    TracePrintf(1, "Lock Release: Lock queue is null\n");
    return ERROR;
  }

  pcb_t* next_pcb =  queuePop(lock->blocked);

  if (next_pcb == NULL) {
    TracePrintf(1, "Lock Release: No one waiting on lock %d, setting to UNLOCKED\n", lock->lock_id);
    lock->owner = UNLOCKED;
    return RELEASE_QUEUE_EMPTY;
  }
  else {
    TracePrintf(1, "Lock Release: Giving ownership of lock %d tp process %d\n", lock->lock_id, next_pcb->pid);
    lock->owner = next_pcb->pid;
    return RELEASE_NEW_OWNER;
  }
}


/******** CVAR FUNCTIONALITY ********/

int cvar_init(ipc_wrapper_t* ipc_wrapper) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Cvar Init: IPC Wrapper is NULL\n");
    return ERROR;
  }

  cvar_t* new_cvar = (cvar_t*) malloc(sizeof(cvar_t));

  if (new_cvar == NULL) {
    TracePrintf(1, "Cvar Init: Couldn't malloc new cvar\n");
    return ERROR;
  }

  new_cvar->cvar_id = ipc_wrapper->next_ipc_id;
  ipc_wrapper->next_ipc_id++;

  queue_t* cvar_queue = queueCreate();

  if (cvar_queue == NULL) {
    TracePrintf(1, "Cvar Init: Failed to create blocked process queue for cvar!\n");
    free(new_cvar);
    return ERROR;
  }

  new_cvar->blocked = cvar_queue;

  return new_cvar->cvar_id;
}

int cvar_signal(ipc_wrapper_t* ipc_wrapper, int cvar_id) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Cvar Signal: IPC Wrapper is NULL\n");
    return ERROR;
  }

  set_t* cvar_set = ipc_get_set(ipc_wrapper, CVAR);

  if (cvar_set == NULL) {
    TracePrintf(1, "Cvar Signal: Cvar set is NULL\n");
    return ERROR;
  }

  cvar_t* cvar = set_find(cvar_set, cvar_id);

  if (cvar == NULL) {
    TracePrintf(1, "Cvar Signal: Cannot find cvar %d\n", cvar_id);
    return ERROR;
  }

  if (cvar->blocked == NULL) {
    TracePrintf(1, "Cvar Signal: Cvar blocked queue is NULL\n");
    return ERROR;
  }

  pcb_t* pcb = (pcb_t*) queuePop(cvar->blocked);

  if (pcb == NULL) {
    TracePrintf(1, "Cvar Signal: No processes to wake up");
    return ERROR;
  }

  return pcb->pid;
}

int* cvar_broadcast(ipc_wrapper_t* ipc_wrapper, int cvar_id) {
  // Check for NULL pointers

  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Cvar Broadcast: IPC Wrapper is NULL\n");
    return NULL;
  }

  set_t* cvar_set = ipc_get_set(ipc_wrapper, CVAR);

  if (cvar_set == NULL) {
    TracePrintf(1, "Cvar Broadcast: Cvar set is NULL\n");
    return NULL;
  }

  cvar_t* cvar = set_find(cvar_set, cvar_id);

  if (cvar == NULL) {
    TracePrintf(1, "Cvar Broadcast: Cannot find cvar %d\n", cvar_id);
    return NULL;
  }

  if (cvar->blocked == NULL) {
    TracePrintf(1, "Cvar Broadcast: Cvar blocked queue is NULL\n");
    return NULL;
  }

  // Allocate memory for array of pids
  int* broadcast_pcbs = (int*) malloc(sizeof(int)*(cvar->blocked->count+1));

  if (broadcast_pcbs == NULL) {
    TracePrintf(1, "Cvar Broadcast: Cannot allocate memory for array of pid's\n");
    return NULL;
  }
  
  pcb_t* pcb = (pcb_t*) queuePop(cvar->blocked);

  if (pcb == NULL) {
    TracePrintf(1, "Cvar Broadcast: No processes to wake up");
    return NULL;
  }

  int i = 0;
  while (pcb != NULL) {
    if (i >= cvar->blocked->count) {
      TracePrintf(1, "Cvar Broadcast: UH OH THIS SHOULD NEVER HAPPEN. Writing past end of malloc'd array\n");
      free(broadcast_pcbs);
      return NULL;
    }
    broadcast_pcbs[i] = pcb->pid;
    i++;
    pcb = (pcb_t*) queuePop(cvar->blocked);
  }

  return broadcast_pcbs;
}

int cvar_wait(ipc_wrapper_t* ipc_wrapper, int cvar_id, int lock_id) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Cvar Wait: IPC Wrapper is NULL\n");
    return ERROR;
  }

  set_t* cvar_set = ipc_get_set(ipc_wrapper, CVAR);

  if (cvar_set == NULL) {
    TracePrintf(1, "Cvar Wait: Cvar set is NULL\n");
    return ERROR;
  }

  cvar_t* cvar = set_find(cvar_set, cvar_id);

  if (cvar == NULL) {
    TracePrintf(1, "Cvar Wait: Cannot find cvar %d\n", cvar_id);
    return ERROR;
  }

  if (cvar->blocked == NULL) {
    TracePrintf(1, "Cvar Wait: Cvar blocked queue is NULL\n");
    return ERROR;
  }

  int lock_release_status = lock_release(ipc_wrapper, lock_id);

  if (lock_release_status == ERROR) {
    return ERROR;
  }
  
  if (queuePush(cvar->blocked, current_process) == ERROR) {
    return ERROR;
  }

  return lock_release_status;
}