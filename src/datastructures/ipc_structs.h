#ifndef _ipc_structs_h
#define _ipc_structs_h

#include <yalnix.h>
#include <hardware.h>
#include <ylib.h>
#include "set.h"
#include "queue.h"
#include "pcb.h"
#include "../kernel.h"

/*********** MACROS ***********/

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

/*********** STRUCTS ***********/

// this is the data structer for all of the Inter Process Coordination handling
// it should be malloc'd by KernelStart and Freed via ipc_wrapper_delete at the end of the kernel's lifecycle
typedef struct ipc_wrapper {
  int next_ipc_id; // the next id for either a lock, cvar, or pipe
  set_t* locks; // set of all existing locks
  set_t* cvars; // set of all existing cvars
  set_t* pipes; // set of all existing pipes
} ipc_wrapper_t;

typedef struct lock {
  int lock_id; // unique identifier
  int owner; // pid of process controlling lock, -1 if unlocked
  queue_t* blocked; // FIFO queue of PCB's waiting for lock
} lock_t;

typedef struct cvar {
  int cvar_id; // unique identifier
  queue_t* blocked; // FIFO queue of PCB's waiting on cvar
} cvar_t;

typedef struct {
	int pipe_id;
	void* buffer;
	int num_bytes_available;
	queue_t* readers;
	set_t* writers;
	int read_available;
	int write_available;
} pipe_t;

/*********** GENERAL FUNCTIONS ***********/

// Allocate memory for and initialize an ipc_wrapper.
// Caller is responsible for calling ipc_wrapper_delte on it
ipc_wrapper_t* ipc_wrapper_init();

// Free memory associated with ipc_wrapper
void ipc_wrapper_delete(ipc_wrapper_t* ipc_wrapper);


// PARAMS: takes in the current ipc wrapper, the types as defined above, and the id of either the lock, cvar, or pipe
// Remove an existing IPC struct from
// Parameters:
//  - ipc_wrapper: existing ipc_wrapper
//  - ipc_id: index of IPC
int ipc_reclaim(ipc_wrapper_t* ipc_wrapper, int ipc_id);


/*********** SPECIFIC LOCK FUNCTIONALITY ***********/

// creates a new lock with no owner
// return ID of the lock on success, ERROR on Failure
int lock_new(ipc_wrapper_t* ipc_wrapper);

// a function for a process to acquire a lock
// if the lock is already owned, it will be added to the queue
// if the lock if free, the process can claim the lock
// returns 0 on success, ERROR on failure
int lock_acquire(ipc_wrapper_t* ipc_wrapper, int lock_id);

// releases the lock as identified by lock_id
// the caller process must own the lock for this to happen
// returns 0 on success, ERROR on failure
int lock_release(ipc_wrapper_t* ipc_wrapper, int lock_id);

// Free memory associated with specified lock
void lock_delete(lock_t* lock);


/*********** CVAR FUNCTIONALITY ***********/

// creates a new cvar with no owner
// returns the ID of the cvar on success, ERROR on failure
int cvar_new(ipc_wrapper_t* ipc_wrapper);

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

// Free memory associated with specified cvar
void cvar_delete(cvar_t* cvar);


/*********** PIPE FUNCTIONALITY ***********/

// creates a new pipe
// returns the ID of the pipe on success, ERROR on failure
int pipe_new(ipc_wrapper_t* ipc_wrapper);

// Write [len] bytes starting at [buf] to the specified pipe.
// return number of bytes written on success, ERROR on failure
int pipe_write(ipc_wrapper_t* ipc_wrapper, int pipe_id, void* buf, int len);

// Read [len] consecutive bytes from the specified pipe into [buf]
// Return number of bytes read on success, ERROR on failure
int pipe_read(ipc_wrapper_t* ipc_wrapper, int pipe_id, void* buf, int len);

// Free memory associated with specified pipe
void pipe_delete(pipe_t* pipe);





#endif /*!_ipc_structs_h*/