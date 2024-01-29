#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"

int kernelFork(void) {
  return 0;
}

int kernelExec(char* filename, char** argvec) {
  return 0;
}

void kernelExit(int status) {
  return 0;
}

int kernelWait(int *status_ptr) {
  return 0;
}

int kernelGetPid(void) {
  return 0;
}

int kernelBrk(void* addr) {
  return 0;
}

int kernelDelay(int clock_ticks) {
  return 0;
}