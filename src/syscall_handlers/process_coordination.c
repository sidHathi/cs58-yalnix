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

  // check for zombies -> 
  // if the head of the zombie linked list is not null
    // iterate through the zombies, and free them entirely -> linked list should now be empty
    // exit immediately
  if (current_process == NULL || current_process->children == NULL || current_process->zombies == NULL) {
    return ERROR;
  }

  if (status_ptr == NULL || !check_memory_validity(status_ptr)) {
    TracePrintf(1, "invalid status pointer passed into wait handler\n");
    return ERROR;
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
    if (status_ptr != NULL) {
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
  memcpy(status_ptr, &current_process->child_exit_status, sizeof(int));
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