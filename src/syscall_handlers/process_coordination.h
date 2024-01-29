#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"

int kernelFork(void);

int kernelExec(char* filename, char** argvec);

void kernelExit(int status);

int kernelWait(int *status_ptr);

int kernelGetPid(void);

int kernelBrk(void* addr);

int kernelDelay(int clock_ticks);