#ifndef _traps_h
#define _traps_h

#include <yalnix.h>
#include <ykernel.h>
#include <hardware.h>
#include "kernel.h"

void TrapKernelHandler(UserContext* user_context);

void TrapClockHandler(UserContext* user_context);

void TrapIllegalHandler(UserContext* user_context);

void TrapMemoryHandler(UserContext* user_context);

void TrapMathHandler(UserContext* user_context);

void TrapTTYReceiveHandler(UserContext* user_context);

void TrapTTYTransmitHandler(UserContext* user_context);

void TrapDiskHandler(UserContext* user_context);

int RegisterTrapHandlers();

#endif /*!_traps_h*/