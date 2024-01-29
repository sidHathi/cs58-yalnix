#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"

int kernelLockInit(int* lock_ipd);

int kernelAcquire(int lock_id);

int kernelRelease(int lock_id);

int kernelCvarInit(int* cvar_ipd);

int kernelCvarSignal(int cvar_ip);

int kernelCvarBroadcast(int cvar_id);

int kernelCvarWait(int cvar_id, int lock_id);

int kernelReclaim(int id);