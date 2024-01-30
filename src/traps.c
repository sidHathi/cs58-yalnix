#include "../../yalnix_framework/include/yalnix.h"
#include "../../yalnix_framework/include/ykernel.h"
#include "../../yalnix_framework/include/hardware.h"

// Trap handler for TRAP_KERNEL
void TrapKernelHandler(UserContext* user_context) {
  // PSEUDOCODE
  // switch statement based on user_context.code
  // call the syscall handler with the corresponding code (use macros from yalnix.h for convenience)
  // the arguments to syscall are found in the registers, starting with regs[0]
  // when done, write the return value of the syscall to the user process in regs[0]
}

// Trap handler for TRAP_CLOCK
void TrapClockHandler(UserContext* user_context) {
  // PSEUDOCODE
  // if there are processes waiting on the ready queue, invoke the scheduler to move the next process to running
  // if there are no processes waiting on the ready queue:
    // if there is a process currently running, do nothing
    // if there is no process currently running, dispatch idle
}

// Trap handler for TRAP_ILLEGAL
void TrapIllegalHandler(UserContext* user_context) {
  // PSEUDOCODE
  // Move the user_context's PCB to the dead PCB array (i.e. abort this process)
  // use traceprintf to tell the user this is happening
}

// Trap handler for TRAP_MEMORY
void TrapMemoryHandler(UserContext* user_context) {
  // PSEUDOCODE
  // if the address that was touched to trigger this process is between the user brk and the bottom of the user stack,
  // increase the size of the user stack to cover this address
  // if the address is below user brk or above the top of the stack, then this was an illegal touching:
    // Move the user_context's PCB to the dead PCB array (i.e. abort this process)
    // use traceprintf to tell the user this is happening
}

// Trap handler for TRAP_MATH
void TrapMathHandler(UserContext* user_context) {
  // PSEUDOCODE
  // Move the user_context's PCB to the dead PCB array (i.e. abort this process)
  // use traceprintf to tell the user this is happening
}

// Trap handler for TRAP_TTY_RECEIVE
void TrapTTYReceiveHandler(UserContext* user_context) {
  // PSEUDOCODE
  // TODO
}

// Trap handler for TRAP_TTY_TRANSMIT
void TrapTTYTransmitHandler(UserContext* user_context) {
  // PSEUDOCODE
  // TODO
}

// NO TrapDiskHandler REQUIRED FOR UNDERGRAD YALNIX