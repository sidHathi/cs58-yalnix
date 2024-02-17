#include <yalnix.h>
#include <ykernel.h>
#include "../kernel.h"
#include "../datastructures/pcb.h"
#include "../datastructures/linked_list.h"
#include "process_coordination.h"

int ForkHandler() {

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

void ExitHandler(UserContext* usr_ctx, int status) {
  // clear necessary memory and free up regions of mem
  // throw error if error in cleanup

  // takes in the exit status, and returns out of the main function calling it

  if (current_process == NULL) {
    return;
  }

  current_process->exit_status = status;
  current_process->state = DEAD;
  pcbOrphanChildren(current_process);
  pcbExit(current_process); // this is weird because the kernel definitely needs to switch contexts -> probabl
  // add check to make sure process is not init -> make this more robust when init is a global
  if (current_process->pid == 1) {
    Halt();
  }

  if (current_process->parent->pid == 1) {
    pcbFree(current_process);
  }
  ScheduleNextProcess(usr_ctx);
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

int GetPidHandler() {
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
  helper_check_heap("Start\n");
  TracePrintf(1, "In brk handler\n");
  TracePrintf(1, "Addr passed: %x\n", addr);
  TracePrintf(1, "Stack pointer: %x\n", current_process->usr_ctx->sp);
  TracePrintf(1, "Current brk: %x\n", current_process->current_brk);
  TracePrintf(1, "Current brk page index: %d\n", ((unsigned int) current_process->current_brk - VMEM_REGION_SIZE)/PAGESIZE);

  // TracePrintf(1, "Showing mappings for region 1 page table:\n");
  // for (int i = 0; i < NUM_PAGES; i++) {
  //   pte_t pte = current_process->page_table[i];
  //   TracePrintf(1, "Page %d. Valid: %d.\n", i, pte.valid);
  // }
  


  // check if brk past user stack
  if (DOWN_TO_PAGE(current_process->usr_ctx->sp) <= (unsigned int)addr) {
    TracePrintf(1, "Error: BrkHandler trying to go above user stack\n");
    return ERROR;
  }
  // check if addr is above or below the current brk
  if ((unsigned int)addr > (unsigned int)current_process->current_brk) {
    int first_invalid_page_index = DOWN_TO_PAGE(current_process->current_brk)/PAGESIZE - NUM_PAGES;
    int addr_page_index = DOWN_TO_PAGE(addr)/PAGESIZE - NUM_PAGES;

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
        TracePrintf(1, "Issue Setting Brk: Trying to allocate already allocated page %d, please check mappings\n", pt_index);
        return ERROR;
      }
	  }	
    current_process->current_brk = (void*) UP_TO_PAGE(addr);
  }
  else if ((unsigned int)addr < (unsigned int)current_process->current_brk) {
    int first_valid_page_index = DOWN_TO_PAGE(current_process->current_brk)/PAGESIZE - 1 - NUM_PAGES;
    int addr_page_index = DOWN_TO_PAGE(addr)/PAGESIZE - NUM_PAGES;

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
	  current_process->current_brk = (void*) UP_TO_PAGE(addr);
	}

  else {
    TracePrintf(1, "No need to move brk\n");
  }

  helper_check_heap("End\n");

  return 0;
}



int DelayHandler(int clock_ticks) {
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
    current_process->delay_ticks = clock_ticks;
    linked_list_push(delayed_pcb_list, current_process);
    
    
    // delay_node_data_t* data = (delay_node_data_t*) malloc(sizeof(delay_node_data_t));
    // data->clock_ticks = clock_ticks;
    // data->pid = current_process->pid;
    // linked_list_push(delayed_pcb_list, data);
    //might need a scheduler call here for the actual delaying of the process that has now been stored in the queue;

    return 0;
  }
  return ERROR;
}