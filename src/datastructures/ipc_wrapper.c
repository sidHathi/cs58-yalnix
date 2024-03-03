#include "ipc_wrapper.h"
#include "../kernel.h"

// Helper function to delete queue of PCB's. This can be passed an an argument to queue_delete
void delete_pcb_queue_helper(void* data, void* arg) {
  pcbFree((pcb_t*) data, (queue_t*) arg);
}

// IPC helper function to find the proper set (locks, cvar, or pipes)
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

  else if (ipc_type == PIPE) {
    return ipc_wrapper->pipes;
  }

  else {
    TracePrintf(1, "No such ipc_type %d\n", ipc_type);
    return NULL;
  }
}


// create a new IPC Wrapper
ipc_wrapper_t* ipc_wrapper_init() {
  ipc_wrapper_t* ipc_wrapper = (ipc_wrapper_t*) malloc(sizeof(ipc_wrapper_t));
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "IPC Wrapper New: Couldn't malloc\n");
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

  return ipc_wrapper;
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


int ipc_reclaim(ipc_wrapper_t* ipc_wrapper, int ipc_id) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "IPC Wrapper: cannot reclaim on null ipc_wrapper\n");
    return ERROR;
  }

  // Find ipc struct within ipc_wrapper struct
  lock_t* lock = (lock_t*) set_pop(ipc_wrapper->locks, ipc_id);
  if (lock != NULL) {
    TracePrintf(1, "IPC Reclaim: removing lock %d\n", lock->lock_id);
    lock_delete(lock);
    return 0;
  }

  cvar_t* cvar = (cvar_t*) set_pop(ipc_wrapper->cvars, ipc_id);
  if (cvar != NULL) {
    TracePrintf(1, "IPC Reclaim: removing cvar %d\n", cvar->cvar_id);
    cvar_delete(cvar);
    return 0;
  }

  // Handle if it's a pipe
  pipe_t* pipe = (pipe_t*) set_pop(ipc_wrapper->pipes, ipc_id);
  if (pipe != NULL) {
    TracePrintf(1, "IPC Reclaim: removing pipe %d\n", pipe->pipe_id);
    pipe_delete(pipe);
    return 0;
  }

  TracePrintf(1, "IPC Reclaim: couldn't find ipc struct with id %d\n", ipc_id);
  return ERROR;
}

/******** LOCK FUNCTIONALITY ********/

int lock_new(ipc_wrapper_t* ipc_wrapper) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Lock New: IPC Wrapper is NULL\n");
    return ERROR;
  }

  if (ipc_wrapper->locks == NULL) {
    TracePrintf(1, "Lock New: ipc_wrapper lock set has not been initialized\n");
    return ERROR;
  }

  if (ipc_wrapper->locks->node_count >= MAX_LOCKS) {
    TracePrintf(1, "Lock New: Maximum locks exceeded\n");
    return ERROR;
  }
  
  lock_t* new_lock = (lock_t*) malloc(sizeof(lock_t));
  
  if (new_lock == NULL) {
    TracePrintf(1, "Lock New: Couldn't malloc new lock\n");
    return ERROR;
  }
  
  new_lock->lock_id = ipc_wrapper->next_ipc_id;
  ipc_wrapper->next_ipc_id++;

  new_lock->owner = UNLOCKED;

  queue_t* lock_queue = queue_new();

  if (lock_queue == NULL) {
    TracePrintf(1, "Lock New: Failed to create blocked process queue for lock!\n");
    free(new_lock);
    return ERROR;
  }

  new_lock->blocked = lock_queue;

  if (set_insert(ipc_wrapper->locks, new_lock->lock_id, new_lock) == ERROR) {
    TracePrintf(1, "Lock New: couldn't insert new lock into existing lock set\n");
    return ERROR;
  }

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
    if(queue_push(lock->blocked, current_process) != 0) {
      TracePrintf(1, "Lock Acquire: Failed to add process to blocked queue\n");
      return ERROR;
    }
    TracePrintf(1, "Lock Acquire: Current process %d added to queue, waiting to acquire lock\n", current_process->pid);
    return ACQUIRE_BLOCKED;
  }
}

//function to release the lock
int lock_release(ipc_wrapper_t* ipc_wrapper, int lock_id) {
  TracePrintf(1, "Lock release function with lock_id %d\n", lock_id);

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

  pcb_t* next_pcb =  queue_pop(lock->blocked);

  if (next_pcb == NULL) {
    TracePrintf(1, "Lock Release: No one waiting on lock %d, setting to UNLOCKED\n", lock->lock_id);
    lock->owner = UNLOCKED;
    return RELEASE_QUEUE_EMPTY;
  }
  else {
    TracePrintf(1, "Lock Release: Giving ownership of lock %d to process %d\n", lock->lock_id, next_pcb->pid);
    lock->owner = next_pcb->pid;
    return RELEASE_NEW_OWNER;
  }
}

void lock_delete(lock_t* lock) {
  if (lock == NULL) {
    return;
  }

  if (lock->blocked != NULL) {
    queue_delete(lock->blocked, free_frame_queue, delete_pcb_queue_helper);
  }

  free(lock);
}


/******** CVAR FUNCTIONALITY ********/

int cvar_new(ipc_wrapper_t* ipc_wrapper) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Cvar New: IPC Wrapper is NULL\n");
    return ERROR;
  }

  if (ipc_wrapper->cvars->node_count >= MAX_CVARS) {
    TracePrintf(1, "Cvar New: Maximum cvar count exceeded\n");
    return ERROR;
  }

  cvar_t* new_cvar = (cvar_t*) malloc(sizeof(cvar_t));

  if (new_cvar == NULL) {
    TracePrintf(1, "Cvar New: Couldn't malloc new cvar\n");
    return ERROR;
  }

  new_cvar->cvar_id = ipc_wrapper->next_ipc_id;
  ipc_wrapper->next_ipc_id++;

  queue_t* cvar_queue = queue_new();

  if (cvar_queue == NULL) {
    TracePrintf(1, "Cvar New: Failed to create blocked process queue for cvar!\n");
    free(new_cvar);
    return ERROR;
  }

  new_cvar->blocked = cvar_queue;

  if (set_insert(ipc_wrapper->cvars, new_cvar->cvar_id, new_cvar) == ERROR) {
    TracePrintf(1, "Cvar New: Failed to insert new cvar into existing set\n");
    free(new_cvar);
    return ERROR;
  }

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

  pcb_t* pcb = (pcb_t*) queue_pop(cvar->blocked);

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
  
  pcb_t* pcb = (pcb_t*) queue_pop(cvar->blocked);

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
    pcb = (pcb_t*) queue_pop(cvar->blocked);
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
  
  if (queue_push(cvar->blocked, current_process) == ERROR) {
    return ERROR;
  }

  return lock_release_status;
}

void cvar_delete(cvar_t* cvar) {
  if (cvar == NULL) {
    return;
  }

  if (cvar->blocked != NULL) {
    queue_delete(cvar->blocked, free_frame_queue, delete_pcb_queue_helper);
  }

  free(cvar);
}

/******** PIPE FUNCTIONALITY ********/

int pipe_new(ipc_wrapper_t* ipc_wrapper) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Pipe New: got null ipc_wrapper\n");
    return ERROR;
  }

  if (ipc_wrapper->pipes->node_count >= MAX_PIPES) {
    TracePrintf(1, "Pipe New: maximum pipe count exceeded\n");
    return ERROR;
  }

  pipe_t* pipe = (pipe_t*) malloc(sizeof(pipe_t));
  if (pipe == NULL) {
    TracePrintf(1, "Pipe New: cannot allocate memory for new pipe\n");
    return ERROR;
  }

  queue_t* readers = queue_new();
  if (readers == NULL) {
    TracePrintf(1, "Pipe New: cannot allocate memory for readers queue\n");
    free(pipe);
    return ERROR;
  }

  set_t* writers = set_new();
  if (writers == NULL) {
    TracePrintf(1, "Pipe New: cannot allocate memory for writers set\n");
    free(pipe);
    queue_delete(readers, NULL, NULL);
    return ERROR;
  }

  pipe->buffer = malloc(sizeof(char) * (PIPE_BUFFER_LEN+1));
  if (pipe->buffer == NULL) {
    TracePrintf(1, "Pipe New: cannot allocate memory for pipe buffer\n");
    free(pipe);
    queue_delete(readers, NULL, NULL);
    set_delete(writers, NULL);
    return ERROR;
  }
  
  pipe->pipe_id = ipc_wrapper->next_ipc_id;
  ipc_wrapper->next_ipc_id++;

  pipe->num_bytes_available = 0;
  pipe->readers = readers;
  pipe->writers = writers;

  if (set_insert(ipc_wrapper->pipes, pipe->pipe_id, pipe) == ERROR) {
    TracePrintf(1, "Pipe New: failed to insert new pipe into existing pipe set\n");
    pipe_delete(pipe);
    return ERROR;
  }
  
  return pipe->pipe_id;
}


// helper function -> 
// shifts the bytes in the buffer left by the specified distance
int
leftshift_buffer(void* buffer, int buffer_size, int dist)
{
  void* tempbuffer = malloc(buffer_size - dist);
  memcpy(tempbuffer, buffer + dist, buffer_size - dist);
  memset(buffer, 0, buffer_size);
  memcpy(buffer, tempbuffer, buffer_size - dist);
}

int pipe_write(ipc_wrapper_t* ipc_wrapper, int pipe_id, void* buf, int len) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Pipe Write: got null ipc_wrapper\n");
    return ERROR;
  }

  pipe_t* pipe = set_find(ipc_wrapper->pipes, pipe_id);

  if (pipe == NULL) {
    TracePrintf(1, "Pipe Write: Cannot find pipe with id %d\n", pipe_id);
    return ERROR;
  }

  // check if the data is from a valid address
  if (!check_memory_validity(buf) || get_raw_page_no(buf) < 128) {
    return -1;
  }
  // check if len is too large
  if (len > PIPE_BUFFER_LEN) {
    return -1;
  }
  // check if buffer is too full
  if (len + pipe->num_bytes_available > PIPE_BUFFER_LEN) {
    set_insert(pipe->writers, current_process->pid, current_process);
  }
  while (len + pipe->num_bytes_available > PIPE_BUFFER_LEN) {
    // block the current process
    current_process->state = BLOCKED;
    ScheduleNextProcess();
  }
  set_pop(pipe->writers, current_process->pid);

  // add the data to the buffer
  void* start_addr = pipe->buffer + pipe->num_bytes_available;
  memcpy(start_addr, buf, len);
  // update the number of available bytes in the buffer
  pipe->num_bytes_available += len;

  // unblock next process waiting to read
  pcb_t* next_reader = queue_pop(pipe->readers);
  if (next_reader != NULL) {
    next_reader->state = READY;
    set_pop(blocked_pcbs, next_reader->pid);
    queue_push(process_ready_queue, next_reader);
  }
  return 0;
}

int pipe_read(ipc_wrapper_t* ipc_wrapper, int pipe_id, void* buf, int len) {
  if (ipc_wrapper == NULL) {
    TracePrintf(1, "Pipe Read: got null ipc_wrapper\n");
    return ERROR;
  }

  pipe_t* pipe = set_find(ipc_wrapper->pipes, pipe_id);

  if (pipe == NULL) {
    TracePrintf(1, "Pipe Read: Cannot find pipe with id %d\n", pipe_id);
    return ERROR;
  }

  // check that the buffer is valid 
  if (buf == NULL || !check_memory_validity(buf) || get_raw_page_no(buf) < 128) {
    return -1;
  }

  // check to see if there are bytes available
  if (pipe->num_bytes_available < 1) {
    // add to waiting queue for pipe readers
    queue_push(pipe->readers, current_process);
    set_insert(blocked_pcbs, current_process->pid, current_process);
    // block until pipe has info
    while (pipe->num_bytes_available  < 1) {
      // block if none available
      current_process->state = BLOCKED;
      ScheduleNextProcess();
    }
  }
  set_pop(blocked_pcbs, current_process->pid);

  // at this point there should be bytes in the pipe to read
  // copy them into the buffer
  int num_bytes_to_copy = MIN(len, pipe->num_bytes_available);
  memcpy(buf, pipe->buffer, num_bytes_to_copy);
  pipe->num_bytes_available -= num_bytes_to_copy;
  // leftshift the buffer
  leftshift_buffer(pipe->buffer, PIPE_BUFFER_LEN, num_bytes_to_copy);
  // if there are still bytes available, unblock next reader
  if (pipe->num_bytes_available > 0) {
    pcb_t* next_reader = queue_pop(pipe->readers);
    TracePrintf(1, "popping pipe reader from read queue -> %d remain\n", pipe->readers->count);
    if (next_reader != NULL) {
      next_reader->state = READY;
      set_pop(blocked_pcbs, next_reader->pid);
      queue_push(process_ready_queue, next_reader);
    }
  }
  // unblock any waiting writers
  for (set_node_t* pcb_node = pipe->writers->head; pcb_node != NULL; pcb_node = pcb_node->next) {
    pcb_t* pcb = pcb_node->item;
    if (pcb != NULL) {
      pcb->state = READY;
      set_pop(blocked_pcbs, pcb->pid);
      queue_push(process_ready_queue, &(pcb->state));
    }
  }

  return 0;
}

void pipe_delete(pipe_t* pipe) {
  free(pipe->buffer);
  queue_delete(pipe->readers, NULL, NULL);
  set_delete(pipe->writers, NULL);
  free(pipe);
}