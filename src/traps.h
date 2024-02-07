#ifndef _traps_h
#define _traps_h

#include <yalnix.h>
#include <ykernel.h>
#include <hardware.h>
#include "kernel.h"

// Trap handler for TRAP_KERNEL
void TrapKernelHandler(UserContext* user_context);

// Trap handler for TRAP_CLOCK
void TrapClockHandler(UserContext* user_context);

// Trap handler for TRAP_ILLEGAL
void TrapIllegalHandler(UserContext* user_context);

// Trap handler for TRAP_MEMORY
void TrapMemoryHandler(UserContext* user_context);

// Trap handler for TRAP_MATH
void TrapMathHandler(UserContext* user_context);

// Trap handler for TRAP_TTY_RECEIVE
void TrapTTYReceiveHandler(UserContext* user_context);

// Trap handler for TRAP_TTY_TRANSMIT
void TrapTTYTransmitHandler(UserContext* user_context);

// Trap handler for TRAP_DISK - should never be invoked
void TrapDiskHandler(UserContext* user_context);

// Standalone function to write address of trap handlers array to priveledged register.
// KernelStart should invoke this function during boot
int RegisterTrapHandlers();

#endif /*!_traps_h*/