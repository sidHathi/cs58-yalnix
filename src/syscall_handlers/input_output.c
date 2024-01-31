#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"



int KernelTtyRead(int tty_id, void* buf, int len) {
  // read input from terminal, set it to buf
  // can only set the max number of bytes <= len from tty_id
  // if there is enough bytes readily availible, set and return
  // else wait until there is a line of input ready to be returned
  //    when there is:
  //      if tty_id > len, then we save the first len bytes, and save remaining bytes to kernel for next read
  //      this should probably be called by this process if this is the case, but not necessary
  //      returns the number of bytes actually copied into the buffer
  //  on any error, return ERROR
  return 0;
}

int KernelTyWrite(int tty_id, void* buf, int len) {
  // len is the length of the buffer
  // process blocked until all bytes are writted to tty_id

  // try to write the bytes to tty_id, if they exceed a limit, do the first amount, then wait for the second amount to be written
  // on success return the number of bytes written
  // on any error return ERROR

  // note it appears that the provided function Tty_Printf uses this function, so probably going to want to make sure this is working from a user standpoint
  return 0;
}