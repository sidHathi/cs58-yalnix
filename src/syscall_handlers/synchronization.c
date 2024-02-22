#include <yalnix.h>
#include <ykernel.h>

int LockInitHandler(int* lock_ipd) {
  /* 
  * STRUCT Lock
  * int owner
  * int id
  * Queue- lock queue for lock to go next
  * */
  // create lock
  // initialize current owner if applicable
  // store lock in lock_ipd
  // return upon success, toss error if ERROR such as lock_ipd not existing

  return 0;
}

int AcquireLockHandler(int lock_id) {
  // use lock_id to locate lock in memory
  // check if there is a current owner of the lock
  // if there is, append user to the queue
  // if not, give the lock directly to the caller and change the owner var in the lock
  // return ERROR if problems arise while this is happening
  return 0;
}

int ReleaseLockHandler(int lock_id) {
  // verify that the lock exists
  // find the lock
  // if the current process is the owner of the lock
  //    then release the lock - remove owner/clear var from struct
  // 
  // check if there are new owners in the queue
  // give the lock to the next owner
  // return ERROR if any of the above checks/attempts fail

  return 0;
}

int CvarInitHandler(int* cvar_ipd) {
  /* 
  * STRUCT cvar
  * int owner
  * int id
  * Queue- lock queue for lock to go next
  * */
  // find the current process in the pcb
  // find a cvar number that has not been used yet (similar to locks)
  // alloc mem and set var to pointer in param.... cvar_ipd
  // set current process to owner
  // set id to found id above
  // return if all of the above successful, throw ERROR upon failures
  return 0;
}

int CvarSignalHandler(int cvar_ip) {
  // locate cvar based on the cvar_ip
  // if its invalid, throw error
  // find current process
  // check if there are other processes in the queue
  // if there are, take the next one out of the queue
  //  make the next one the current owner, and let that process resume from its wait

  // if all sucessful return, if there were no processes waiting, do nothing

  // throw ERROR where necessary during checks
  return 0;
}

int CvarBroadcastHandler(int cvar_id) {
  // locate cvar based on the cvar_ip
  // if its invalid, throw error
  // find current process
  // check if there are other processes in the queue
  // then free all of those processes, from their respective pcbs in memory
  // return

  //throw ERROR where necessary during checks

  // note this is pretty much the same as signal, except instead of removing one from the queue, we are dequeueing the entire queue.
  // this means there will need to be more code handling in the surrounding code on the wait.
  return 0;
}

int CvarWaitHandler(int cvar_id, int lock_id) {
  // cvar is the id of the svar that the process is waiting on
  // lock_id is the lock that its waiting on

  // verify cvar exists
  // verify lock exists

  // release the lock- so that other processes can use while its waiting

  // make the process wait and enqueue it in the cvar's queue

  // once the var is free aquire the lock and return

  // throw ERROR where necessary during the checks
  return 0;
}

int ReclaimHandler(int id) {
  // check if its a Lock, Cvar, or a Pipe
  // locate each one respectively
  // verify that they exist
  // Make sure that there are no processes waiting on the lock/cvar/pipe, throw ERROR if not
  // free the lock/cvar/pipe
  // dealloc the mem for it
  // return respectively

  // throw ERROR on checks where necesarry
  // the above pseudo code is for one, this would in theory would need to be repeated and changed slightly for each cvar/pipe/lock

  return 0;
}