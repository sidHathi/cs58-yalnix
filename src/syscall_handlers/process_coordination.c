#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"
#include "cs58-yalnix/src/kernel.h"
#include "../datastructures/pcb.h"
#include "../datastructures/linked_list.h"


int ForkHandler(void) {

  // create new child process and pcb for child process

  // if child process has been successfully created

  //  check for child process code to ensure its the child accessing the code i.e pid = 0
  //    returns 0
  //  else if its the parent process
  //    returns new ProcessID

  // If no successful creation, return ERROR

  return 0;
}

int ExecHandler(char* filename, char** argvec, int argc) {
  
  // clear current process memory
  // execute filename argvec[1]...argvec[n]
  // if exec new program fails, return Error if parent process has not been destroyed
  return 0;
}

void ExitHandler(int status) {
  // clear necessary memory and free up regions of mem
  // throw error if error in cleanup

  // takes in the exit status, and returns out of the main function calling it

}

int WaitHandler(int *status_ptr) {
  // unlock the lock of the status so that another process can use
  // get the PID and exit status returned by a child process of the calling program
  
  // if calling process has a child who has exited, return with that information

  // else if calling process has no children, return Error

  // else:
  //  while waiting our turn for the status - waiting till next child exits or is aborted
  //      chill
  //      if status or cond that we are waiting for is found,
  //          grab the lock, lock the process,
  //          return so that process can continue in the code. - child PID should be returned
  //          ** if the status_ptr is not null, just set the Child PID to that pointer

  return 0;
}

int GetPidHandler(void) {
  // find the pcb for this process
  // index into the pcb and find the where the PID is stored- don't know the exact location yet
  // return pid
  return ((pcb_t *)(current_process->pid));
}

int KernelBrk(void* addr) {
  // find the lowest point of memory in the stack
  // set this point to addr
  // note that this pointshould be rounded up to the next multiple of PAGESIZE bytes.
  // should alloc and dealloc the necessary amount of mem for the address

  // if addr is invalid, return ERROR
  // otherwise return 0 on success

  // PAGESHIFT;
  // DOWN_TO_PAGE;

  // check if brk into or past user stack
  if (DOWN_TO_PAGE(current_process->usr_ctx->sp) >> PAGESHIFT <= (unsigned int)addr) {
    TracePrintf(1, "Error: KernelBrk trying to go into user stack\n");
    return ERROR;
  }

  


  return 0;
}

int DelayHandler(int clock_ticks) {
  // sleep for the number of clock ticks provided on this current process only
  // return and allow process to continue after the delay
  // on success should return 0
  // return ERROR if clock_ticks is less than 0, or time is improperly carried out

  if(clock_ticks == 0) {
    return 0;
  }
  else if (clock_ticks < 0) {
    TracePrintf(1, "Clock ticks found to be less than 0!\n");
    return ERROR;
  }
  else {
    // make the current process sleep for the number of clock ticks.
    DelayNode_t* delay_node;
    delay_node->clock_ticks = clock_ticks;
    delay_node->process = current_process;

    liniked_list_push(delay_list, delay_node);

    //might need a scheduler call here for the actual delaying of the process that has now been stored in the queue;

    return 0;
  }
  return ERROR;
}