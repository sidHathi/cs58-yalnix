#include "ipc.h"

int PipeInitHandler(int* pipe_idp) {

  // initialize pipe
  // if error return ERROR
  // if invalid pointer, return ERROR
  // upon success, set pipe identifier to pipe_idp
  // return 0 on success
  // check validity of pipe_idp pointer
  if (!check_memory_validity(pipe_idp) || get_raw_page_no(pipe_idp) < 128) {
    return ERROR;
  }

  TracePrintf(1, "Initializing new pipe\n");
  int new_pipe_id = pipe_new(ipc_wrapper);
  if (new_pipe_id == ERROR) {
    TracePrintf(1, "pipe initialization failed\n");
    return ERROR;
  }

  *pipe_idp = new_pipe_id;

  return 0;
}

int PipeReadHandler(int pipe_id, void* buf, int len) {
  // find pipe
  // read len of pipe bytes into the users buffer
  // buf + len = next free space after pipe is added
  // if pipe is empty, block the caller
  // else if pipe length <= len unread butes, give them all to the caller and return
  // else if pipe length > len unready butes, give the first len butes to caller and return. Retain the unread bytes in the pipe for later
  // else throw an ERROR and return.
  return pipe_read(ipc_wrapper, pipe_id, buf, len);
}

int PipeWriteHandler(int pipe_id, void* buf, int len) {
  // find the pipe
  // write the number of bytes (len) in buffer to the pipe
  // going to want to use append here, as the pipe works in FIFO order
  // return 0 upon success immmediately
  // return ERROR if anything doesn't go correctly
  return pipe_write(ipc_wrapper, pipe_id, buf, len);
}