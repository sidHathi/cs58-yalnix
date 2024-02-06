#include "../../yalnix_framework/include/yalnix.h"
#include "../../yalnix_framework/include/ykernel.h"
#include "../../yalnix_framework/include/hardware.h"
#include "traps.h"
#include "kernel.h"

// Trap handler for TRAP_KERNEL
void TrapKernelHandler(UserContext* user_context) {
  // PSEUDOCODE
  // switch statement based on user_context.code
  // call the syscall handler with the corresponding code (use macros from yalnix.h for convenience)
  // the arguments to syscall are found in the registers, starting with regs[0]
  // when done, write the return value of the syscall to the user process in regs[0]

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap Kernel! Code: %x\n", user_context->code);
}

// Trap handler for TRAP_CLOCK
void TrapClockHandler(UserContext* user_context) {
  // PSEUDOCODE
  // if there are processes waiting on the ready queue, invoke the scheduler to move the next process to running
  // if there are no processes waiting on the ready queue:
    // if there is a process currently running, do nothing
    // if there is no process currently running, dispatch idle

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap Clock!\n");
}

// Trap handler for TRAP_ILLEGAL
void TrapIllegalHandler(UserContext* user_context) {
  // PSEUDOCODE
  // Move the user_context's PCB to the dead PCB array (i.e. abort this process)
  // use traceprintf to tell the user this is happening

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap Illegal! This trap is not yet handled!\n");
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
  TracePrintf(1, "Trap Memory! This trap is not yet handled!\n");
}

// Trap handler for TRAP_MATH
void TrapMathHandler(UserContext* user_context) {
  // PSEUDOCODE
  // Move the user_context's PCB to the dead PCB array (i.e. abort this process)
  // use traceprintf to tell the user this is happening

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap Math! This trap is not yet handled!\n");
}

// Trap handler for TRAP_TTY_RECEIVE
void TrapTTYReceiveHandler(UserContext* user_context) {
  // PSEUDOCODE
  // Get terminal index from user_context.code
  // Use hardware operation TtyReceive to get input from terminal.
  // Copy terminal value into tty_buffers[user_context.code]
  // This value is available for user process to copy via TtyRead

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap TTY Receive! This trap is not yet handled!\n");
}

// Trap handler for TRAP_TTY_TRANSMIT
void TrapTTYTransmitHandler(UserContext* user_context) {
  // PSEUDOCODE
  // Complete blocked process that started this terminal output
  // If there is a next terminal output, display it

  // Checkpoint 2 functionality:
  TracePrintf(1, "Trap TTY Transmit! This trap is not yet handled!\n");
}

void TrapDiskHandler(UserContext* user_context) {
  TracePrintf(1, "Trap Disk! This funcionality is not required for Yalnix!\n");
}

int RegisterTrapHandlers() {
  if (!virtual_mem_enabled) {
    TracePrintf(1, "Cannot initialize interrupt vector table before virtual memory is enabled.\n");
    return ERROR;
  }

  void** function_pointers = (void**) malloc(TRAP_VECTOR_SIZE * sizeof(void*));
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