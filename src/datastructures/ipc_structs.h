#ifndef _ipc_structs_h
#define _ipc_structs_h

#include <yalnix.h>
#include <hardware.h>
#include <ylib.h>
#include "set.h"
#include "queue.h"

#define LOCK 0
#define CVAR 1
#define PIPE 2

#define MAX_LOCKS 16
#define MAX_CVARS 16
#define MAX_PIPES 16

#define UNLOCKED -1

#define ACQUIRE_SUCCESS 0 // No PCB Movement
#define ACQUIRE_BLOCKED 1 // Syscall handler handles PCB Movement

#define RELEASE_QUEUE_EMPTY 0
#define RELEASE_NEW_OWNER 1

//this is the data structer for all of the Inter Process Coordination handling
typedef struct ipc_wrapper {
  int next_ipc_id; // the next id for either a lock, cvar, or pipe

  set_t* locks; // set of all existing locks
  set_t* cvars; // set of all existing cvars
  set_t* pipes; // set of all existing pipes

} ipc_wrapper_t;

//helper functions for creating and deleting the ipc_wrapper
ipc_wrapper_t* ipc_wrapper_init();
void ipc_wrapper_delete(ipc_wrapper_t* ipc_wrapper);

// PARAMS: takes in the current ipc wrapper, the types as defined above, and the id of either the lock, cvar, or pipe
int ipc_reclaim(ipc_wrapper_t* ipc_wrapper, int ipc_type, int ipc_id);


/*********** LOCK FUNCTIONALITY ***********/

typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  queue_t* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

// creates a new lock with no owner
// return ID of the lock on success, ERROR on Failure
int lock_init(ipc_wrapper_t* ipc_wrapper);

// a function for a process to acquire a lock
// if the lock is already owned, it will be added to the queue
// if the lock if free, the process can claim the lock
// returns 0 on success, ERROR on failure
int lock_acquire(ipc_wrapper_t* ipc_wrapper, int lock_id);

// releases the lock as identified by lock_id
// the caller process must own the lock for this to happen
// returns 0 on success, ERROR on failure
int lock_release(ipc_wrapper_t* ipc_wrapper, int lock_id);


/*********** CVAR FUNCTIONALITY ***********/

typedef struct cvar {
  int cvar_id; // unique identifier
  queue_t* blocked; // FIFO queue of PCB's waiting on cvar
} cvar_t;

// creates a new cvar with no owner
// returns the ID of the cvar on success, ERROR on failure
int cvar_init(ipc_wrapper_t* ipc_wrapper);

// function to signal the condition variable specified by cvar_id
// return pid of the process to unblock on success, ERROR on failure
int cvar_signal(ipc_wrapper_t* ipc_wrapper, int cvar_id);

// function to broadcast the condition variable specified by cvar_id
// returns malloc'd array of pid's to unblock on success, returns NULL on failure
// Caller is responsible for freeing this array
int* cvar_broadcast(ipc_wrapper_t* ipc_wrapper, int cvar_id);

// function that releases the lock specified by lock_id
// and then waits on the cvar specified by cvar_id
// when the process wakes up it re-acquires the lock
// returns ERROR on failure, returns return-value of release_lock on success
int cvar_wait(ipc_wrapper_t* ipc_wrapper, int cvar_id, int lock_id);


/*********** PIPE FUNCTIONALITY ***********/

typedef struct {
	int id;
	void* buffer;
	int num_bytes_available;
	queue_t* readers;
	set_t* writers;
	int read_available;
	int write_available;
} pipe_t;



#endif /*!_ipc_structs_h*/