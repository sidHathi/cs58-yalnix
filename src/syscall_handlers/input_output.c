#include "../../../yalnix_framework/include/yalnix.h"
#include "../../../yalnix_framework/include/ykernel.h"
#include "kernel.h"


// HELPER FUNCTION:
// returns an allocated array of size 2
// first bit contains memory region of input addr
// second bit contains the page number in the
// corresponding page table
int
get_raw_page_no(void* addr)
{
  int page_no = (unsigned int)addr/PAGESIZE;
  return page_no;
}

int TtyReadHandler(int tty_id, void* buf, int len, UserContext* usr_ctx) {
  // read input from terminal, set it to buf
  // can only set the max number of bytes <= len from tty_id
  // if there is enough bytes readily availible, set and return
  // else wait until there is a line of input ready to be returned
  //    when there is:
  //      if tty_id > len, then we save the first len bytes, and save remaining bytes to kernel for next read
  //      this should probably be called by this process if this is the case, but not necessary
  //      returns the number of bytes actually copied into the buffer
  //  on any error, return ERROR

  /*
    - read next line from specified terminal, up some maximum `len` byte
    - if the line is already present, this might return immediately with the bytes
    - if not, it will block until the bytes are present
    - if more bytes are present than `len` , remaining bytes stay in terminal buffer for next read

    - when `TtyRead` is called for a process, the address of the buffer should be stored in `current_process->tty_buffer` , the length of the read should be stored in `tty_read_len`, and `tty_has_bytes` should be 0
    - When the TtyTrap is thrown in the kernel, it should have a list of waiting pcbs, and what terminal they’re waiting on → maybe iterate through the blocked pcb list and check `tty_terminal_requested` and `tty_wating`?
    - The scheduler needs to interpret the tty information in the blocked pcb list every time it runs →
  */


  // perform safety checks:
  // buf needs to be valid region 1 memory
  // tty id needs to be valid
  if (!check_memory_validity(buf) || get_raw_page_no < 128 || get_raw_page_no > 255) {
    TracePrintf(1, "Buffer passed into TtyReadHandler is invalid or lives in region 0");
    return ERROR;
  }
  
  if (tty_id > NUM_TERMINALS < tty_id < 0) {
    TracePrintf(1, "Invalid tty id passed into TtyReadHandler\n");
    return ERROR;
  }

  // check to see if bytes are immediately available to read?
  if (current_tty_state->bytes_available[tty_id] > 0) {
    // in this case, immediately consume the available bytes and return
    tty_buffer_consume(current_tty_state, buf, tty_id, len);
    return 0;
  }

  // otherwise, this process is blocked and waiting -> should invoke scheduler after blocking imo
  current_process->state = BLOCKED;
  current_process->tty_num_bytes_requested = len;
  current_process->tty_read_buffer_r1 = buf;
  linked_list_push(current_tty_state->curr_readers[tty_id], current_process);

  ScheduleNextProcess(usr_ctx);
  // at this point we expect the bytes to have been copied into the pcb r0 buffer
  // only thing left to do is move r0 buffer to r1 and return 
  if (!check_memory_validity(current_process->tty_read_buffer_r0) || current_process->tty_read_buffer_r0 == NULL) {
    return ERROR;
  }
  memcpy(buf, current_process->tty_read_buffer_r0, current_process->tty_num_bytes_read);

  return 0;
}

int TtyWriteHandler(int tty_id, void* buf, int len, UserContext* usr_ctx) {
  // len is the length of the buffer
  // process blocked until all bytes are writted to tty_id

  // try to write the bytes to tty_id, if they exceed a limit, do the first amount, then wait for the second amount to be written
  // on success return the number of bytes written
  // on any error return ERROR

  // note it appears that the provided function Tty_Printf uses this function, so probably going to want to make sure this is working from a user standpoint

  /*
    - blocks caller
    - when the terminal is available to write, do the write instruction
        - When is a terminal available to write? → when a write/read instruction on that terminal has just completed → in that case we need to have a queue of processes waiting on each terminal
        - when an operation completes, if a process is waiting on a certain terminal, we need to unblock it
    - when hardware throws trap to indicate that the write is complete, unblock the caller
    - kernel needs to use a region 0 buffer to complete the write, even when user passes in data from region 1 (security issue)?
    - if TtyWrite wants to write more bytes than the operation can handle at once, it needs to break the operation down into multiple chunks
        - How does this happen? → while loop basically → write → store how many bytes have been written in a local variable on the kernel stack → invoke the scheduler → when the scheduler returns to the function → write the next `TERMINAL_MAX_LINE` bytes and check whether any bytes are left → if there are none left, exit the while loop and return to the caller
  */

  // check that the parameters are valid
  if (!check_memory_validity(buf) || get_raw_page_no(buf) < 128 || tty_id < 0 || tty_id > NUM_TERMINALS) {
    TracePrintf(1, "invalid input passed into TtyWriteHandler\n");
    return ERROR;
  }
  // check if the requested terminal is available to write -> block otherwise
  if (!current_tty_state->availability[tty_id]) {
    current_process->state = BLOCKED;
    current_process->tty_write_waiting = 1;
    queuePush(current_tty_state->write_queues[tty_id], current_process);
  }
  while (!current_tty_state->availability[tty_id]) {
    ScheduleNextProcess(usr_ctx);
  }
  // at this point the assumption is that the requested terminal is now available
  // copy the data in the buffer into a region 0 location (malloc, memcpy)
  void* r0_write_buffer = malloc(sizeof(char) * len);
  // mark the current process as the current writer for the buffer
  current_tty_state->curr_writers[tty_id] = current_process;
  // now we have to invoke tty transmit in blocks
  // we can only transmit TERMINAL_MAX_LINE bytes at a time
  // so if len is greater than that value it needs to be broken up
  for (int start_byte = 0; start_byte < len/TERMINAL_MAX_LINE; start_byte += TERMINAL_MAX_LINE) {
    int transmit_size = MIN(len, TERMINAL_MAX_LINE);
    TtyTransmit(tty_id, (void*) (r0_write_buffer + start_byte), transmit_size);
    // block and invoke scheduler until write finishes
    current_process->state = BLOCKED;
    current_process->tty_write_waiting = 1;
    ScheduleNextProcess(usr_ctx);
  }
  
  // this should return when the write operation is finished

  // return success to user

  return 0;
}