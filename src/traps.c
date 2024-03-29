#include <yalnix.h>
#include <ykernel.h>
#include <hardware.h>
#include "traps.h"
#include "kernel.h"
#include "syscall_handlers/process_coordination.h"
#include "syscall_handlers/input_output.h"
#include "syscall_handlers/synchronization.h"
#include "syscall_handlers/ipc.h"

// Trap handler for TRAP_KERNEL
void TrapKernelHandler(UserContext* user_context) {
  TracePrintf(1, "Entered TRAP KERNEL with code %x\n", user_context->code);
  int rc;

  memcpy(current_process->usr_ctx, user_context, sizeof(UserContext));

  // Invoke appropriate syscall handler
  switch (user_context->code) {
    case YALNIX_FORK:
      TracePrintf(1, "TRAP KERNEL: invoking fork syscall handler\n");
      rc = ForkHandler();
      break;
    case YALNIX_EXEC:
      TracePrintf(1, "TRAP KERNEL: invoking exec syscall handler with filename %s", (char*) user_context->regs[0]);
      rc = ExecHandler((char*) user_context->regs[0], (char**) user_context->regs[1]);
      break;
    case YALNIX_EXIT:
      TracePrintf(1, "TRAP KERNEL: invoking exit syscall handler with status %d\n", user_context->regs[0]);
      ExitHandler(user_context->regs[0]);
      rc = user_context->regs[0];
      break;
    case YALNIX_WAIT:
      TracePrintf(1, "TRAP KERNEL: invoking wait syscall handler with address %x\n", user_context->regs[0]);
      if ((unsigned int) user_context->regs[0] < VMEM_0_LIMIT) {
        TracePrintf(1, "Security Risk in Trap Kernel: User trying to pass address in region 0. Returning ERROR\n");
        rc = ERROR;
      }
      else {
        rc = WaitHandler((int*) user_context->regs[0]);
      }
      break;
    case YALNIX_GETPID:
      TracePrintf(1, "TRAP KERNEL: invoking getpid syscall handler\n");
      rc= GetPidHandler();
      break;
    case YALNIX_BRK:
      TracePrintf(1, "TRAP KERNEL: invoking brk syscall handler with addr %p\n", (void*) user_context->regs[0]);
      rc = BrkHandler((void*) user_context->regs[0]);
      break;
    case YALNIX_DELAY:
      TracePrintf(1, "TRAP KERNEL: invoking delay syscall handler with %d ticks\n", user_context->regs[0]);
      rc = DelayHandler((int) user_context->regs[0]);
      break;
    case YALNIX_TTY_READ:
      TracePrintf(1, "TRAP KERNEL: invoking tty read syscall handler on terminal %d\n", user_context->regs[0]);
      rc = TtyReadHandler((int) user_context->regs[0], (void*) user_context->regs[1], (int) user_context->regs[2]);
      break;
    case YALNIX_TTY_WRITE:
      TracePrintf(1, "TRAP KERNEL: invoking tty write syscall handler on terminal %d\n", user_context->regs[0]);
      rc = TtyWriteHandler((int) user_context->regs[0], (void*) user_context->regs[1], (int) user_context->regs[2]);
      break;
    case YALNIX_PIPE_INIT:
      TracePrintf(1, "TRAP KERNEL: invoking pipe init syscall handler with identifier %d\n", *(int*)(user_context->regs[0]));
      rc = PipeInitHandler((int*) user_context->regs[0]);
      break;
    case YALNIX_PIPE_READ:
      TracePrintf(1, "TRAP KERNEL: invoking pipe read syscall handler on pipe %d\n", user_context->regs[0]);
      rc = PipeReadHandler((int) user_context->regs[0], (void*) user_context->regs[1], (int) user_context->regs[2]);
      break;
    case YALNIX_PIPE_WRITE:
      TracePrintf(1, "TRAP KERNEL: invoking pipe write syscall handler on pipe %d\n", user_context->regs[0]);
      rc = PipeWriteHandler((int) user_context->regs[0], (void*) user_context->regs[1], (int) user_context->regs[2]);
      break;
    case YALNIX_LOCK_INIT:
      TracePrintf(1, "TRAP KERNEL: invoking lock init syscall handler with identifier %d\n", *(int*)user_context->regs[0]);
      if ((unsigned int) user_context->regs[0] < VMEM_0_LIMIT) {
        TracePrintf(1, "Security Risk in Trap Kernel: User trying to pass address in region 0. Returning ERROR\n");
        rc = ERROR;
      }
      else {
        rc = LockInitHandler((int*) user_context->regs[0]);
      }
      break;
    case YALNIX_LOCK_ACQUIRE:
      TracePrintf(1, "TRAP KERNEL: invoking lock acquire syscall handler with identifier %d\n", user_context->regs[0]);
      rc = AcquireLockHandler((int) user_context->regs[0]);
      break;
    case YALNIX_LOCK_RELEASE:
      TracePrintf(1, "TRAP KERNEL: invoking lock release syscall handler with identifier %d\n", (int) user_context->regs[0]);
      rc = ReleaseLockHandler((int) user_context->regs[0]);
      break;
    case YALNIX_CVAR_INIT:
      TracePrintf(1, "TRAP KERNEL: invoking cvar init syscall handler with identifier %d\n", (int)user_context->regs[0]);
      if ((unsigned int) user_context->regs[0] < VMEM_0_LIMIT) {
        TracePrintf(1, "Security Risk in Trap Kernel: User trying to pass address in region 0. Returning ERROR\n");
        rc = ERROR;
      }
      else {
        rc = CvarInitHandler((int*) user_context->regs[0]);
      }
      break;
    case YALNIX_CVAR_SIGNAL:
      TracePrintf(1, "TRAP KERNEL: invoking cvar signal syscall handler on cvar %d\n", user_context->regs[0]);
      rc = CvarSignalHandler((int) user_context->regs[0]);
      break;
    case YALNIX_CVAR_BROADCAST:
      TracePrintf(1, "TRAP KERNEL: invoking cvar broadcast syscall handler on cvar %d\n", user_context->regs[0]);
      rc = CvarBroadcastHandler((int) user_context->regs[0]);
      break;
    case YALNIX_CVAR_WAIT:
      TracePrintf(1, "TRAP KERNEL: invoking cvar wait syscall handler with on cvar %d and lock %d\n", user_context->regs[0], user_context->regs[1]);
      rc = CvarWaitHandler((int) user_context->regs[0], (int) user_context->regs[1]);
      break;
    case YALNIX_RECLAIM:
      TracePrintf(1, "TRAP KERNEL: invoking reclaim syscall handler on lock %d\n", user_context->regs[0]);
      rc = ReclaimHandler((int) user_context->regs[0]);
      break;
    default:
      TracePrintf(1, "Oops! Invalid Trap Kernel Code %x\n", user_context->code);
      Halt();
  }

  if (current_process != NULL) {
    current_process->usr_ctx->regs[0] = rc;
    memcpy(user_context, current_process->usr_ctx, sizeof(UserContext));
  } else {
    ScheduleNextProcess();
  }
}


// Trap handler for TRAP_CLOCK
void TrapClockHandler(UserContext* user_context) {
  TracePrintf(1, "Trap Clock!\n");
  memcpy(current_process->usr_ctx, user_context, sizeof(UserContext));

  // Decrement delay count for all delayed processes. If any get to 0,  move them to the ready queue
  set_node_t* node = delayed_pcbs->head;
  int pid;
  pcb_t* pcb;
  int i = 1;
  while (node != NULL) {
    pcb = (pcb_t*) (node->item);
    pid = pcb->pid;
    pcb->delay_ticks--;
    node = node->next;
    if (pcb->delay_ticks > 0) {
    }
    else {
      set_pop(delayed_pcbs, pid);
      pcb->state = READY;
      queue_push(process_ready_queue, pcb);
    }
  }

  // Invoke Scheduler
  ScheduleNextProcess();
  if (current_process != NULL) {
    memcpy(user_context, current_process->usr_ctx, sizeof(UserContext));
  } else {
    ScheduleNextProcess();
  }
}

// Trap handler for TRAP_ILLEGAL
void TrapIllegalHandler(UserContext* user_context) {
  TracePrintf(1, "Trap Illegal.\n");
  ExitHandler(ERROR);
  ScheduleNextProcess();
}

// Trap handler for TRAP_MEMORY
void TrapMemoryHandler(UserContext* user_context) {
  // PSEUDOCODE
  // if the address that was touched to trigger this process is between the user brk and the bottom of the user stack,
  // increase the size of the user stack to cover this address
  // if the address is below user brk or above the top of the stack, then this was an illegal touching:
    // Move the user_context's PCB to the dead PCB array (i.e. abort this process)
    // use traceprintf to tell the user this is happening

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap Memory!\n");
  TracePrintf(1, "offending addr: %x program counter: %x\n", user_context->addr, user_context->pc);
  // find the lowest point of memory in the stack
  // set this point to addr
  // note that this pointshould be rounded up to the next multiple of PAGESIZE bytes.
  // should alloc and dealloc the necessary amount of mem for the address
  unsigned int addr = (unsigned int) user_context->addr;
  helper_check_heap("Start\n");
  TracePrintf(1, "In Memory handler\n");
  TracePrintf(1, "Addr passed: %x\n", addr);
  TracePrintf(1, "Stack pointer: %x\n", current_process->usr_ctx->sp);
  TracePrintf(1, "Current brk: %x\n", current_process->current_brk);
  TracePrintf(1, "Current brk page index: %d\n", ((unsigned int) current_process->current_brk - VMEM_REGION_SIZE)/PAGESIZE);

  // check if brk past user stack
  if (DOWN_TO_PAGE(current_process->usr_ctx->sp) <= (unsigned int)addr) {
    TracePrintf(1, "Error: Trap Memory trying to go above user stack\n");
    ExitHandler(ERROR);
    ScheduleNextProcess();
    return;
  }
  // check if addr is above or below the current brk
  if ((unsigned int)addr > (unsigned int)current_process->current_brk + PAGESIZE && addr < VMEM_1_BASE + VMEM_REGION_SIZE) {
    int first_invalid_page_index = NUM_PAGES - 1;
    int addr_page_index = DOWN_TO_PAGE(addr)/PAGESIZE - NUM_PAGES;

    for(int pt_index = first_invalid_page_index; pt_index >= addr_page_index; pt_index--) {	
	    
	    if(current_process->page_table[pt_index].valid == 0) {		    
		    int* allocated_frame = (int*) queue_pop(free_frame_queue);

		    if(allocated_frame == NULL) {
          ExitHandler(ERROR);
          ScheduleNextProcess();
          return;
        }
        TracePrintf(1, "Trap Memory: Allocating page %d\n", pt_index);
        current_process->page_table[pt_index].valid = 1;
        current_process->page_table[pt_index].prot = PROT_READ | PROT_WRITE;
        current_process->page_table[pt_index].pfn = *allocated_frame;
        free(allocated_frame);
	    }
      WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
	  }	
  }
  else {
    ExitHandler(ERROR);
    ScheduleNextProcess();
  }
}

// Trap handler for TRAP_MATH
void TrapMathHandler(UserContext* user_context) {
  TracePrintf(1, "Trap Math.\n");
  ExitHandler(ERROR);
  ScheduleNextProcess();
}

// Trap handler for TRAP_TTY_RECEIVE
void TrapTTYReceiveHandler(UserContext* user_context) {
  // this trap needs to:
  // Call TtyReceive to move the data from the terminal into the current
  // terminal state buffer
  // call tty_handle_received
  int tty_id = user_context->code;
  TracePrintf(1, "Trap tty recieve called with tty_id %d\n", tty_id);
  if (tty_id < 0 || tty_id > NUM_TERMINALS) {
    TracePrintf(1, "invalid tty_id found in Trap Tty Tranmit\n");
    return;
  }

  int receipt_len = TtyReceive(tty_id, current_tty_state->buffers[tty_id], TERMINAL_MAX_LINE);
  tty_handle_received(current_tty_state, tty_id, receipt_len);
  if (current_process != NULL) {
    memcpy(user_context, current_process->usr_ctx, sizeof(UserContext));
  } else {
    ScheduleNextProcess();
  }
}

// Trap handler for TRAP_TTY_TRANSMIT
void TrapTTYTransmitHandler(UserContext* user_context) {
  // this trap needs to:
  // figure out which process is currently writing
  // unblock it
  // allow it to continue writing
  int tty_id = user_context->code;
  TracePrintf(1, "Trap tty transmmit called with tty id %d\n", tty_id);
  if (tty_id < 0 || tty_id > NUM_TERMINALS) {
    TracePrintf(1, "invalid tty_id found in Trap Tty Tranmit\n");
    return;
  }

  if (current_tty_state == NULL) {
    TracePrintf(1, "Trap Tty Tranmit called before tty state init\n");
    return;
  }

  pcb_t* writing_pcb = current_tty_state->curr_writers[tty_id];
  if (writing_pcb == NULL) {
    // mark terminal as available and continue
    current_tty_state->availability[tty_id] = 1;
  } else {
    writing_pcb->state = READY;
    writing_pcb->tty_write_waiting = 0;
    set_pop(blocked_pcbs, writing_pcb->pid);
    queue_push(process_ready_queue, writing_pcb);
  }

  if (current_process != NULL) {
    memcpy(user_context, current_process->usr_ctx, sizeof(UserContext));
  } else {
    ScheduleNextProcess();
  }
}

void TrapDiskHandler(UserContext* user_context) {
  TracePrintf(1, "Trap Disk! This funcionality is not required for Yalnix!\n");
  memcpy(user_context, current_process->usr_ctx, sizeof(UserContext));
}

int RegisterTrapHandlers() {
  if (!virtual_mem_enabled) {
    TracePrintf(1, "Cannot initialize interrupt vector table before virtual memory is enabled.\n");
    return ERROR;
  }

  void** function_pointers = (void**) malloc(TRAP_VECTOR_SIZE * sizeof(void*));
  if (function_pointers == NULL) {
        TracePrintf(1, "Register Trap Handlers: Failed to mallloc function pointers!\n");
        return ERROR;
  }
  function_pointers[TRAP_KERNEL] = (void*) &TrapKernelHandler;
  function_pointers[TRAP_CLOCK] = (void*) &TrapClockHandler;
  function_pointers[TRAP_ILLEGAL] = (void*) &TrapIllegalHandler;
  function_pointers[TRAP_MEMORY] = (void*) &TrapMemoryHandler;
  function_pointers[TRAP_MATH] = (void*) &TrapMathHandler;
  function_pointers[TRAP_TTY_RECEIVE] = (void*) &TrapTTYReceiveHandler;
  function_pointers[TRAP_TTY_TRANSMIT] = (void*) &TrapTTYTransmitHandler;
  function_pointers[TRAP_DISK] = (void*) &TrapDiskHandler;
  WriteRegister(REG_VECTOR_BASE, (unsigned int) function_pointers);

  return 0;
}