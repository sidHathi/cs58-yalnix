#include "input_output.h"

int TtyReadHandler(int tty_id, void* buf, int len) {
  /*
    - read next line from specified terminal, up some maximum `len` bytes
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
  TracePrintf(1, "Checking buffer validity\n");
  if (buf == NULL || !check_memory_validity(buf) || get_raw_page_no(buf) < 128 || get_raw_page_no(buf) > 255 || current_tty_state == NULL) {
    TracePrintf(1, "Buffer passed into TtyReadHandler is invalid or lives in region 0");
    return ERROR;
  }
  
  if (tty_id > NUM_TERMINALS < tty_id < 0) {
    TracePrintf(1, "Invalid tty id passed into TtyReadHandler\n");
    return ERROR;
  }

  TracePrintf(1, "Checking TtyRead buffer availability\n");
  // check to see if bytes are immediately available to read?
  if (current_tty_state->bytes_available[tty_id] > 0) {
    // in this case, immediately consume the available bytes and return
    TracePrintf(1, "Invoking tty buffer consume\n");
    tty_buffer_consume(current_tty_state, buf, tty_id, len);
    return 0;
  }

  TracePrintf(1, "Blocking process in TtyRead\n");
  // otherwise, this process is blocked and waiting -> should invoke scheduler after blocking imo
  current_process->state = BLOCKED;
  current_process->tty_num_bytes_requested = len;
  current_process->tty_read_buffer_r1 = buf;
  set_insert(current_tty_state->curr_readers[tty_id], current_process->pid, current_process);

  ScheduleNextProcess();
  // at this point we expect the bytes to have been copied into the pcb r0 buffer
  // only thing left to do is move r0 buffer to r1 and return 
  if (!check_memory_validity(current_process->tty_read_buffer_r0) || current_process->tty_read_buffer_r0 == NULL) {
    return ERROR;
  }
  memcpy(buf, current_process->tty_read_buffer_r0, current_process->tty_num_bytes_read);
  set_pop(current_tty_state->curr_readers[tty_id], current_process->pid);

  return 0;
}

int TtyWriteHandler(int tty_id, void* buf, int len) {
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

  TracePrintf(1, "Tty write handler called with buffer length %d\n", len);
  // check if the requested terminal is available to write -> block otherwise
  while (!current_tty_state->availability[tty_id]) {
    current_process->state = BLOCKED;
    current_process->tty_write_waiting = 1;
    queue_push(current_tty_state->write_queues[tty_id], current_process);
    ScheduleNextProcess();
  }
  TracePrintf(1, "Tty write handler writing\n");
  // at this point the assumption is that the requested terminal is now available
  // copy the data in the buffer into a region 0 location (malloc, memcpy)
  void* r0_write_buffer = malloc(sizeof(char) * len);
  if (r0_write_buffer == NULL) {
    TracePrintf(1, "TTY Write Handler: Failed to mallloc!\n");
    return ERROR;
  }
  memcpy(r0_write_buffer, buf, len);
  // mark the current process as the current writer for the buffer
  current_tty_state->curr_writers[tty_id] = current_process;
  current_tty_state->availability[tty_id] = 0;
  // now we have to invoke tty transmit in blocks
  // we can only transmit TERMINAL_MAX_LINE bytes at a time
  // so if len is greater than that value it needs to be broken up
  for (int start_byte = 0; start_byte < len; start_byte += TERMINAL_MAX_LINE) {
    int transmit_size = MIN(len - start_byte, TERMINAL_MAX_LINE);
    TracePrintf(1, "Tty write handler invoking TtyTransmit\n");
    current_tty_state->curr_writers[tty_id] = current_process;
    TtyTransmit(tty_id, (void*) (r0_write_buffer + start_byte), transmit_size);
    // block and invoke scheduler until write finishes
    current_process->state = BLOCKED;
    current_process->tty_write_waiting = 1;
    // add current process to writing queue
    ScheduleNextProcess();
    TracePrintf(1, "Tty write handler unblocked in iteration %d. max iteration is %d\n", start_byte, (len-1)/TERMINAL_MAX_LINE);
  }
  // this should return when the write operation is finished
  current_process->tty_write_waiting = 0;
  
  // at this point the terminal should also be marked as available
  current_tty_state->availability[tty_id] = 1;
  current_tty_state->curr_writers[tty_id] = NULL;

  // the next process on the writing queue should be unblocked
  pcb_t* next_writer = queue_pop(current_tty_state->write_queues[tty_id]);
  if (next_writer != NULL) {
    next_writer->state = READY;
    next_writer->tty_write_waiting = 0;
    set_pop(blocked_pcbs, next_writer->pid);
    queue_push(process_ready_queue, next_writer);
    // maybe change this -> the rationale is that waiting processes need to
    // write immediately after they're unblocked, or else there's a chance
    // that they'll wake up and the current process will already have started
    // writing again -> maybe talk about this in OH
    ScheduleNextProcess();
  }

  // return success to user
  return 0;
}