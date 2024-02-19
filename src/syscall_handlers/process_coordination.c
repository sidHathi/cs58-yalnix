#include <yalnix.h>
#include <ykernel.h>
#include "../kernel.h"
#include "../datastructures/pcb.h"
#include "../datastructures/linked_list.h"
#include "process_coordination.h"

// helper function for pcb list management:
// removes the pcb with pid `pid` from the `list`
// of pcb_t pointers
void
pcb_list_remove(linked_list_t* list, int pid) {
  if (list == NULL) {
    return;
  }
  lnode_t* curr = list->front;
  while (curr != NULL) {
    lnode_t* next = curr->next;
    if (((pcb_t*) (curr->data))->pid == pid) {
      if (curr->prev != NULL) {
        curr->prev->next = curr->next;
      } else {
        list->front = curr->next;
      }
      free(curr);
      curr = next;
      break;
    }
  }
}

int ForkHandler() {

  // create new child process and pcb for child process

  // if child process has been successfully created

  //  check for child process code to ensure its the child accessing the code i.e pid = 0
  //    returns 0
  //  else if its the parent process
  //    returns new ProcessID

  // If no successful creation, return ERROR



//  - According to doc → very similar process to cloning init into idle for checkpoint 3
// - Allocate stack frames
// - allocate region 1 page table

// - allocate frames for new region 1 pages → any valid entry index in the current page table gets a frame in the new page table
// - create new process pid
// - create a new pcb given the new pid, page table, stack frames + user context for current process
//     - add the pcb to the children linked list of the current pcb
// - Use `KCCopy` to copy the kernel context into the new pcb and populate the kernel stack frames with data
// - manipulate the userland stacks of both the parent process and the newly created process so that the return value from the fork syscall in the parent is the newpid for the process and the return value for the forked process is 0 → idk how to do this honestly
// - put the process on the ready queue
// - exit

  TracePrintf(1, "Fork Handler: Entering the System Fork Handler\n");

  
  pte_t* curr_pte;
  unsigned int curr_page_index;

  // allocate region 1 page table:
  pte_t* new_page_table = (pte_t*) malloc(sizeof(pte_t) * NUM_PAGES);
  for (int i = 0; i < NUM_PAGES; i ++) {
    new_page_table[i].valid = 0; // all invalid to start
  }
  // - allocate frames for new region 1 pages → any valid entry index in the current page table gets a frame in the new page table

  for (int i = 0; i < MAX_PT_LEN; i++) {
    curr_pte = &(current_process->page_table[i]); // current page table entry
    curr_page_index = (i + MAX_PT_LEN); //current region 1 page number


    if(curr_pte->valid == 1) {

      int* allocated_frame_region1 = (int*) queuePop(free_frame_queue);

      if(allocated_frame_region1 == NULL) {
        return ERROR;
      }
      new_page_table[curr_page_index].valid = 1;
      new_page_table[curr_page_index].prot = PROT_READ | PROT_WRITE;
      new_page_table[curr_page_index].pfn = allocated_frame_region1;

    }
  } 
  // create new process pid
  // - create a new pcb given the new pid, page table, stack frames + user context for current process
  //     - add the pcb to the children linked list of the current pcb

  int new_pid = helper_new_pid(new_page_table);

  pcb_t* new_pcb = pcbNew(new_pid, new_page_table, NULL, current_process, current_process->usr_ctx, NULL);

  if(new_pcb == NULL) {
    TracePrintf(1, "Fork: newPCB is NULL!\n");
    return ERROR;
  }

  linked_list_push(current_process->children, new_pcb);

  // Use `KCCopy` to copy the kernel context into the new pcb and populate the kernel stack frames with data
  KernelContextSwitch(&KCCopy, new_pcb, NULL);

  if (current_process->pid == new_pid) {
    TracePrintf(1, "Fork: Child returning\n");
    queuePush(process_ready_queue, new_pcb);
    return 0;
  }
  else {
    TracePrintf(1, "Fork: Parent returning: %d\n", new_pid);
    return new_pid;
  }
  

  
// - manipulate the userland stacks of both the parent process and the newly created process so that the return value from the fork syscall in the parent is the newpid for the process and the return value for the forked process is 0 → idk how to do this honestly


// - put the process on the ready queue
// - exit

}

int ExecHandler(char* filename, char** argvec) {

  TracePrintf(1, "ExecHandler: executing with parameters: %s, %d\n", filename, argvec[0]);
  TracePrintf(1, "ExecHandler: Calling Load Program\n");
  int rc = LoadProgram(filename, argvec, current_process);
  TracePrintf(1, "ExecHandler: Finished Load Program\n");
  
  if (rc == KILL) {
    ExitHandler(KILL);
  }
  else if (rc == ERROR) {
    TracePrintf(1, "ExecHandler: Exiting with ERROR\n");
    return ERROR;
  }

  return 0;
}

void ExitHandler(int status) {
  // clear necessary memory and free up regions of mem
  // throw error if error in cleanup

  // takes in the exit status, and returns out of the main function calling it

  if (current_process == NULL) {
    return;
  }

  current_process->exit_status = status;
  current_process->state = DEAD;
  pcbOrphanChildren(current_process);
  pcbExit(current_process); // Current pcb can now hold only cursory data about the process and is functionally dead
  // add check to make sure process is not init -> make this more robust when init is a global
  if (current_process->pid == init_process->pid) {
    Halt(); // program halts when init exits
  }

  // this process now needs to become a zombie of its parent rather than a child -> involves removing itself from children list and appending to zombies
  pcb_t* parent_pcb = current_process->parent;
  if (parent_pcb == NULL) {
    return;
  }
  // remove from children list
  pcb_list_remove(parent_pcb->children, current_process->pid);

  // if the parent of this process is waiting -> unblock it and add it to ready queue -> if not, add to zombies list
  if (parent_pcb->waiting) {
    parent_pcb->state = READY;
    queuePush(process_ready_queue, parent_pcb);
    num_ready_processes ++;

    // remove parent from blocked process list
    pcb_list_remove(blocked_pcb_list, parent_pcb->pid);
    num_blocked_processes --;
    
    // free current pcb
    pcbFree(current_process);
    current_process = NULL;
  } else {
    // add process as zombie
    linked_list_push(parent_pcb->zombies, current_process);
    // add to dead process list
    linked_list_push(dead_pcb_list, current_process);
    num_dead_processes ++;
  }
  parent_pcb->child_exit_status = status;
}

int WaitHandler(UserContext* usr_ctx, int *status_ptr) {
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


  if (current_process == NULL || current_process->children == NULL || current_process->zombies == NULL) {
    TracePrintf(1, "current process is not valid in wait handler\n");
    return ERROR;
  }

  // check to make sure the status pointer is not null and valid -> 
  // if it isn't, we just won't use it
  int shouldStoreStatus = 1;
  if (status_ptr == NULL || !check_memory_validity(status_ptr)) {
    TracePrintf(1, "invalid status pointer passed into wait handler\n");
    shouldStoreStatus = 0;
  }

  // if wait is called while the process has already exited zombie children,
  // it should remove the zombies, free their pcbs, and exit immediately
  if (current_process->zombies != NULL && current_process->zombies->front != NULL) {
    lnode_t* curr_zombie_node = current_process->zombies->front;
    int exit_status = ((pcb_t*)current_process->zombies->front->data)->exit_status;

    // empty zombie list
    while (curr_zombie_node != NULL) {
      // remove zombie from the dead process list
      int zombie_pid = ((pcb_t*) curr_zombie_node->data)->pid;
      pcb_list_remove(dead_pcb_list, zombie_pid);
      num_dead_processes --;

      // free associated data and move to next node
      lnode_t* next_zombie_node = curr_zombie_node->next;
      pcbFree((pcb_t*)curr_zombie_node->data);
      free(curr_zombie_node);
      curr_zombie_node = next_zombie_node;
    }
    // mark list as emptied
    current_process->zombies->front = current_process->zombies->front = NULL;
    if (shouldStoreStatus) {
      memcpy(status_ptr, &exit_status, sizeof(int));
    }

    return 0;
  }

  // check to make sure there are children ->
    // if none return error val
  if (current_process->children->front == NULL) {
    return ERROR;
  }
  // Otherwise, update the status values in the pcb
  current_process->waiting = 1;
  current_process->state = BLOCKED;
  // don't return until the current process is unblocked
  while (current_process->waiting && current_process->state == BLOCKED) {
    ScheduleNextProcess(usr_ctx);
    // the scheduler should return here when we switch back to the waiting process
  }
  // set status pointer
  if (shouldStoreStatus) {
    memcpy(status_ptr, &current_process->child_exit_status, sizeof(int));
  }
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
  if(clock_ticks == 0) {
    return 0;
  }
  else if (clock_ticks < 0) {
    TracePrintf(1, "Clock ticks found to be less than 0!\n");
    return ERROR;
  }
  else {
    // make the current process sleep for the number of clock ticks.
    TracePrintf(1, "Pushing process %d to delay list\n", current_process->pid);
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