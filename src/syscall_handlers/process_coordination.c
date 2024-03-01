#include <yalnix.h>
#include <ykernel.h>
#include <unistd.h>
#include "../kernel.h"
#include "../datastructures/pcb.h"
#include "../datastructures/set.h"
#include "process_coordination.h"
#include "../datastructures/set.h"

// helper function for pcb list management:
// removes the pcb with pid `pid` from the `list`
// of pcb_t pointers
// void
// pcb_list_remove(linked_list_t* list, int pid) {
//   if (list == NULL) {
//     return;
//   }
//   lnode_t* curr = list->front;
//   while (curr != NULL) {
//     lnode_t* next = curr->next;
//     if (((pcb_t*) (curr->data))->pid == pid) {
//       if (curr->prev != NULL) {
//         curr->prev->next = curr->next;
//         if (curr->next != NULL) {
//           curr->next->prev = curr->prev;
//         } else {
//           list->rear = curr->prev;
//         }
//       } else {
//         list->front = curr->next;
//         if (list->front != NULL) {
//           list->front->prev = NULL;
//         } else {
//           list->rear = NULL;
//         }
//       }
//       // pcbFree((pcb_t*)curr->data, free_frame_queue);
//       free(curr);
//       break;
//     }
//     curr = next;
//   }
// }

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
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

  // allocate region 1 page table:
  pte_t* new_page_table = (pte_t*) malloc(sizeof(pte_t) * NUM_PAGES);
  for (int i = 0; i < NUM_PAGES; i ++) {
    new_page_table[i].valid = 0; // all invalid to start
  }
  TracePrintf(1, "Fork Handler: new page table initialized\n");
  // - allocate frames for new region 1 pages → any valid entry index in the current page table gets a frame in the new page table

  for (int i = 0; i < NUM_PAGES; i++) {
    pte_t curr_pte = current_process->page_table[i]; // current page table entry
    // TracePrintf(1, "Fork Handler: pte contents for index %d: valid: %d, pfn: %d\n", i, curr_pte.valid, curr_pte.pfn);
    if (!curr_pte.valid) {
      continue;
    }

    // TracePrintf(1, "Fork Handler: copying page %d\n", i);
    int* allocated_frame_region1 = (int*) queue_pop(free_frame_queue);
    if (allocated_frame_region1 == NULL) {
      TracePrintf(1, "Fork Handler: No more free frames availible\n");
      return ERROR;
    }
    int frame_no = *allocated_frame_region1;
    free(allocated_frame_region1);
    // TracePrintf(1, "Fork Handler: allocating frame no %d\n", frame_no);

    // set the page validity and protections in the new page table
    new_page_table[i].valid = 1;
    new_page_table[i].prot = curr_pte.prot;
    new_page_table[i].pfn = frame_no;
    // TracePrintf(1, "Fork Handler: set new page table validity and frame\n");

    // temporarily link the page 2 below the kernel stack to the newly allocated frame
    int idx = NUM_PAGES - 2*NUM_KSTACK_FRAMES;
    void* dest_addr = (void*) (KERNEL_STACK_BASE - KERNEL_STACK_MAXSIZE);
    void* src_addr = (void*) (VMEM_REGION_SIZE + i*PAGESIZE);
    // TracePrintf(1, "Fork Handler: dest addr: %p, src addr: %p, r0 idx: %d, ropages: %p\n", dest_addr, src_addr, idx, region_0_pages);
    region_0_pages[idx].valid = 1;
    region_0_pages[idx].prot = PROT_READ | PROT_WRITE;
    region_0_pages[idx].pfn = frame_no;
    // TracePrintf(1, "Fork Handler: assigned page %d to new frame\n", idx);

    // reset the region 0 tlb
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    // copy memory into the frame (now assigned to page idx)
    memcpy(dest_addr, src_addr, PAGESIZE);
    // TracePrintf(1, "Fork Handler: copied data to new frame\n", idx);

    // reset the region 0 page table entry for idx
    region_0_pages[idx].valid = 0;
    region_0_pages[idx].prot = 0;
    region_0_pages[idx].pfn = 0;
    // TracePrintf(1, "Fork Handler: dereferenced kernel page\n", idx);
  }

  // create new process pid
  // - create a new pcb given the new pid, page table, stack frames + user context for current process
  //     - add the pcb to the children linked list of the current pcb
  int new_pid = helper_new_pid(new_page_table);
    TracePrintf(1, "Fork Handler: new page table pid: %d\n", new_pid);

  pcb_t* new_pcb = pcbNew(new_pid, new_page_table, NULL, current_process, current_process->usr_ctx, NULL);
  new_pcb->current_brk = current_process->current_brk;

  int* kernel_stack_1_p = (int*)queue_pop(free_frame_queue);
  int* kernel_stack_2_p = (int*)queue_pop(free_frame_queue);
  if (kernel_stack_1_p == NULL || kernel_stack_2_p == NULL) {
    TracePrintf(1, "No available frames for kernel stack\n");
    return ERROR;
  }
  new_pcb->kernel_stack_pages[0].valid = 1;
  new_pcb->kernel_stack_pages[0].prot = PROT_READ | PROT_WRITE;
  new_pcb->kernel_stack_pages[0].pfn = *kernel_stack_1_p;
  new_pcb->kernel_stack_pages[1].valid = 1;
  new_pcb->kernel_stack_pages[1].prot = PROT_READ | PROT_WRITE;
  new_pcb->kernel_stack_pages[1].pfn = *kernel_stack_2_p;
  free(kernel_stack_1_p);
  free(kernel_stack_2_p);

  if (new_pcb == NULL) {
    TracePrintf(1, "Fork: newPCB is NULL!\n");
    return ERROR;
  }

  TracePrintf(1, "Fork Handler: adding new pcb to current process children\n");
  if (current_process->children == NULL) {
    current_process->children = set_new();
  }
  set_insert(current_process->children, new_pcb->pid, new_pcb);

  // add the process to the ready queue
  TracePrintf(1, "Fork Handler: adding new pcb to ready queue\n");
  queue_push(process_ready_queue, new_pcb);
  // Use `KCCopy` to copy the kernel context into the new pcb and populate the kernel stack frames with data
  TracePrintf(1, "Fork Handler: executing kccopy\n");
  KernelContextSwitch(&KCCopy, new_pcb, NULL);

  // return the correct values
  if (current_process->pid == new_pid) {
    TracePrintf(1, "Fork: Child returning\n");
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
    return 0;
  } else {
    TracePrintf(1, "Fork: Parent returning: %d\n", new_pid);
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
    return new_pid;
  }
}

int ExecHandler(char* filename, char** argvec) {
  if (filename == NULL || current_process == NULL) {
    TracePrintf(1, "invalid args passed to Exec\n");
    return ERROR;
  }

  // Check existence of executable
  if (access(filename, F_OK) == -1) {
    TracePrintf(1, "Exec Handler could not find file %s\n", filename);
    return ERROR;
  }

  // Ensure file is executable
  if (access(filename, X_OK) == -1) {
    TracePrintf(1, "%s is not an executable\n", filename);
    return ERROR;
  }

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

  TracePrintf(1, "Entering exit handler \n");
  TracePrintf(1, "Pcb page table is null: %d\n", current_process->page_table == NULL);
  TracePrintf(1, "Free frame queue is null: %d\n", free_frame_queue == NULL);
  if (current_process == NULL) {
    TracePrintf(1, "Exit handler: current process is null\n");
    return;
  }

  TracePrintf(1, "Exit handler: marking current process as dead\n");
  current_process->exit_status = status;
  current_process->state = DEAD;
  helper_retire_pid(current_process->pid);
  TracePrintf(1, "Exit handler: orphaning process children\n");
  pcbOrphanChildren(current_process);
  TracePrintf(1, "Exit handler: freeing pcb data\n");
  // pcbExit(current_process, free_frame_queue); // Current pcb can now hold only cursory data about the process and is functionally dead
  // add check to make sure process is not init -> make this more robust when init is a global
  if (current_process->pid == init_process->pid) {
    TracePrintf(1, "Exit handler: current process is init: exiting\n");
    Halt(); // program halts when init exits
  }

  // this process now needs to become a zombie of its parent rather than a child -> involves removing itself from children list and appending to zombies
  pcb_t* parent_pcb = current_process->parent;
  if (parent_pcb == NULL) {
    TracePrintf(1, "Exit handler: process has a null parent\n");
    return;
  }
  // remove from children list
  set_pop(parent_pcb->children, current_process->pid);

  // if the parent of this process is waiting -> unblock it and add it to ready queue -> if not, add to zombies list
  if (parent_pcb->waiting) {
    TracePrintf(1, "Exit handler: parent process waiting -> unblocking\n");
    parent_pcb->state = READY;
    queue_push(process_ready_queue, parent_pcb);

    // remove parent from blocked process list
    set_pop(blocked_pcbs, parent_pcb->pid);
    
    // free current pcb
    // TracePrintf(1, "Exit handler: freeing current pcb\n");
    if (current_process != NULL) {
      TracePrintf(1, "Exit handler: freeing current pcb\n");
      pcbFree(current_process, free_frame_queue);
      current_process = NULL;
    }
  } else {
    // add process as zombie
    set_insert(parent_pcb->zombies, current_process->pid, current_process);
    // add to dead process list
    set_insert(dead_pcbs, current_process->pid, current_process);
  }
  pcbExit(current_process, free_frame_queue);
  current_process = NULL;
  parent_pcb->child_exit_status = status;
  TracePrintf(1, "Leaving exit handler\n");
}

int WaitHandler(int *status_ptr) {
  if (current_process == NULL || current_process->children == NULL || current_process->zombies == NULL) {
    TracePrintf(1, "current process is not valid in wait handler\n");
    return ERROR;
  }

  // check to make sure the status pointer is not null and valid -> 
  // if it isn't, we just won't use it
  int shouldStoreStatus = 1;
  if (status_ptr == NULL || !check_memory_validity(status_ptr)) {
    shouldStoreStatus = 0;
  }

  // if wait is called while the process has already exited zombie children,
  // it should remove the zombies, free their pcbs, and exit immediately
  if (current_process->zombies != NULL && current_process->zombies->head != NULL) {
    TracePrintf(1, "WaitHandler: child has already exited -> removing zombies and returning \n");
    set_node_t* curr_zombie_node = current_process->zombies->head;
    int exit_status = ((pcb_t*)current_process->zombies->head->item)->exit_status;

    // empty zombie list
    while (curr_zombie_node != NULL) {
      // remove zombie from the dead process list
      int zombie_pid = ((pcb_t*) curr_zombie_node->item)->pid;
      pcb_t* zombie_pcb = set_pop(dead_pcbs, zombie_pid);
      if (zombie_pcb != NULL) {
        pcbFree(zombie_pcb, free_frame_queue);
      }

      // free associated data and move to next node
      set_node_t* next_zombie_node = curr_zombie_node->next;
      helper_retire_pid(((pcb_t*)curr_zombie_node->item)->pid);
      pcbFree((pcb_t*)curr_zombie_node->item, free_frame_queue);
      free(curr_zombie_node);
      curr_zombie_node = next_zombie_node;
    }
    // mark list as emptied
    current_process->zombies->head = current_process->zombies->head = NULL;
    if (shouldStoreStatus) {
      memcpy(status_ptr, &exit_status, sizeof(int));
    }

    return 0;
  }

  // check to make sure there are children ->
    // if none return error val
  if (current_process->children->head == NULL) {
    TracePrintf(1, "WaitHandler: current process has no children -> returning \n");
    return ERROR;
  }
  // Otherwise, update the status values in the pcb
  current_process->waiting = 1;
  current_process->state = BLOCKED;
  TracePrintf(1, "WaitHandler: blocking current process \n");
  // don't return until the current process is unblocked
  while (current_process->waiting && current_process->state == BLOCKED) {
    TracePrintf(1, "WaitHandler: invoking scheduler \n");
    ScheduleNextProcess(current_process->usr_ctx);
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
		    int* allocated_frame = (int*) queue_pop(free_frame_queue);

		    if(allocated_frame == NULL) {
          return ERROR;
        }
        TracePrintf(1, "BrkHandler: Allocating page %d\n", pt_index);
        current_process->page_table[pt_index].valid = 1;
        current_process->page_table[pt_index].prot = PROT_READ | PROT_WRITE;
        current_process->page_table[pt_index].pfn = *allocated_frame;
        free(allocated_frame);
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
        queue_push(free_frame_queue, &curr_frame_number);
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
    TracePrintf(1, "In delay handler: pid of current_proccess = %d\n", current_process->pid);
    current_process->delay_ticks = clock_ticks;
    current_process->state = DELAYED;
    ScheduleNextProcess();
    return 0;
  }
  return ERROR;
}