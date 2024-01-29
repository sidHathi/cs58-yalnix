#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"

int kernelLockInit(int* lock_ipd) {
  return 0;
}

int kernelAcquire(int lock_id) {
  return 0;
}

int kernelRelease(int lock_id) {
  return 0;
}

int kernelCvarInit(int* cvar_ipd) {
  return 0;
}

int kernelCvarSignal(int cvar_ip) {
  return 0;
}

int kernelCvarBroadcast(int cvar_id) {
  return 0;
}

int kernelCvarWait(int cvar_id, int lock_id) {
  return 0;
}

int kernelReclaim(int id) {
  return 0;
}