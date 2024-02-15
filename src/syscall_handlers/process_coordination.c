#include <yalnix.h>
#include <ykernel.h>
#include "../kernel.h"
#include "../datastructures/pcb.h"
#include "../datastructures/linked_list.h"
#include "process_coordination.h"


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

int ExecHandler(char* filename, char** argvec) {
  
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
  return current_process->pid;
}

int BrkHandler(void* addr) {
  // find the lowest point of memory in the stack
  // set this point to addr
  // note that this pointshould be rounded up to the next multiple of PAGESIZE bytes.
  // should alloc and dealloc the necessary amount of mem for the address

  // check if brk past user stack
  if (DOWN_TO_PAGE(current_process->usr_ctx->sp) >> PAGESHIFT <= (unsigned int)addr) {
    TracePrintf(1, "Error: BrkHandler trying to go above user stack\n");
    return ERROR;
  }
  // check if addr is above or below the current brk
  if ((unsigned int)addr >= current_process->current_brk) {
    int first_invalid_page_index = DOWN_TO_PAGE(current_process->current_brk)/PAGESIZE;
    int addr_page_index = DOWN_TO_PAGE(addr)/PAGESIZE;

    for(int pt_index = first_invalid_page_index; pt_index <= addr_page_index; pt_index++) {	
	    
	    if(current_process->page_table[pt_index].valid == 0) {		    
		    int* allocated_frame = (int*) queuePop(free_frame_queue);

		    if(allocated_frame == NULL) {
          return ERROR;
        }
        TracePrintf(1, "BrkHandler: Allocating page %d\n", pt_index);
        current_process->page_table[pt_index].valid = 1;
        current_process->page_table[pt_index].prot = PROT_READ | PROT_WRITE;
        current_process->page_table[pt_index].pfn = *allocated_frame;
	    }
      else {
        TracePrintf(1, "Issue Setting Brk: Trying to allocate already allocated page, please check mappings\n");
        return ERROR;
      }
	  }	
    current_process->current_brk = UP_TO_PAGE(addr);
  }
  else {
    int first_valid_page_index = DOWN_TO_PAGE(current_process->current_brk)/PAGESIZE - 1;
    int addr_page_index = DOWN_TO_PAGE(addr)/PAGESIZE;

    for (int pt_index = first_valid_page_index; pt_index >= addr_page_index; pt_index--) {
      
      if(current_process->page_table[pt_index].valid = 1) {
        region_0_pages[pt_index].valid = 0;
        unsigned int curr_frame_number = region_0_pages[pt_index].pfn;
        queuePush(free_frame_queue, &curr_frame_number);
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
      }
      else {
        TracePrintf(1, "Issue Setting Brk: Trying to dealloc pages that weren't allocated in the first place\n");
        return ERROR;
      }
    }
	  current_process->current_brk = UP_TO_PAGE(addr);
	}

  return 0;
}



int KernelDelay(int clock_ticks) {
  // sleep for the number of clock ticks provided on this current process only
  // return and allow process to continue after the delay
  // on success should return 0
  // return ERROR if clock_ticks is less than 0, or time is improperly carried out
  TracePrintf(1, "Here %d\n", clock_ticks);
  if(clock_ticks == 0) {
    return 0;
  }
  else if (clock_ticks < 0) {
    TracePrintf(1, "Clock ticks found to be less than 0!\n");
    return ERROR;
  }
  else {
    // make the current process sleep for the number of clock ticks.
    delay_node_data_t* data = (delay_node_data_t*) malloc(sizeof(delay_node_data_t));
    data->clock_ticks = clock_ticks;
    data->pid = current_process->pid;

    linked_list_push(delay_list, data);

    //might need a scheduler call here for the actual delaying of the process that has now been stored in the queue;

    return 0;
  }
  return ERROR;
}