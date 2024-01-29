#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"

int kernelTyRead(int tty_id, void* buf, int len);

int kernelTtyWrite(int tty_id, void* buf, int len);