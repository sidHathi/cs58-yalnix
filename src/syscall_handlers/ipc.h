#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"

int kernelPipeInit(int* pipe_idp);

int kernelPipeRead(int pipe_id, void* buf, int len);

int kernelPipeWrite(int pipe_id, void* buf, int len);