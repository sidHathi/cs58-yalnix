#ifndef _synchronization_h
#define _synchronization_h

#include <yalnix.h>
#include <ykernel.h>
#include "../datastructures/ipc_wrapper.h"
#include "../kernel.h"

/*
* LockInit:
*
* Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
*/
int LockInitHandler(int* lock_idp);


/*
* Acquire:
*
* Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
*/
int AcquireLockHandler(int lock_id);


/*
* Release:
*
* Release the lock identified by lock id. The caller must currently hold this lock. In case of any error, the value
* ERROR is returned.
*/
int ReleaseLockHandler(int lock_id);


/*
* CvarInit:
*
* Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is
* returned.
*/
int CvarInitHandler(int* cvar_idp);


/*
* CvarSignal:
*
* Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value
* ERROR is returned.
*/
int CvarSignalHandler(int cvar_id);


/*
* CvarBroadcast: 
*
* Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the
* value ERROR is returned.
*/
int CvarBroadcastHandler(int cvar_id);


/*
* CvarWait:
*
* The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified
* by cvar id. When the kernel-level process wakes up (e.g., because the condition variable was signaled), it
* re-acquires the lock. (Use Mesa-style semantics.)
* When the lock is finally acquired, the call returns to userland.
* In case of any error, the value ERROR is returned.
*/
int CvarWaitHandler(int cvar_id, int lock_id);


/*
* Reclaim:
*
* Destroy the lock, condition variable, or pipe indentified by id, and release any associated resources. In case of
* any error, the value ERROR is returned.
* If you feel additional specification is necessary to handle unusual scenarios, then create and document it.
*/
int ReclaimHandler(int id);


#endif