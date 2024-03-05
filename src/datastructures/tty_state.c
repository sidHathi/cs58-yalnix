#include "tty_state.h"
#include "hardware.h"
#include "kernel.h"

tty_state_t*
tty_state_init()
{
  tty_state_t* tty_state = malloc(sizeof(tty_state_t));
  if (tty_state == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }

  tty_state->availability = malloc(sizeof(int) * NUM_TERMINALS);
  if (tty_state->availability == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }
  
  tty_state->curr_writers = malloc(sizeof(pcb_t*) * NUM_TERMINALS);
  if (tty_state->curr_writers == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }

  tty_state->curr_readers = malloc(sizeof(set_t*) * NUM_TERMINALS);
  if (tty_state->curr_readers == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }

  tty_state->buffers = malloc(sizeof(char*) * NUM_TERMINALS);
  if (tty_state->buffers == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }

  tty_state->bytes_available = malloc(sizeof(int) * NUM_TERMINALS);
  if (tty_state->bytes_available == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }

  tty_state->write_queues = malloc(sizeof(queue_t*) * NUM_TERMINALS);
  if (tty_state->write_queues == NULL) {
    TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
    return NULL;
  }
  for (int i = 0; i < NUM_TERMINALS; i ++) {
    tty_state->availability[i] = 1; // all terminals initially available
    tty_state->curr_writers[i] = NULL;
    tty_state->curr_readers[i] = set_new();
    tty_state->buffers[i] = malloc(sizeof(char) * TERMINAL_MAX_LINE);
    if (tty_state->buffers[i] == NULL) {
      TracePrintf(1, "TTY State Init: Failed to mallloc!\n");
      return NULL;
    }
    tty_state->bytes_available[i] = 0;
    tty_state->write_queues[i] = queue_new();
  }

  return tty_state;
}

void
tty_handle_received(tty_state_t* tty_state, int tty_id, int num_bytes)
{
  if (tty_state == NULL || tty_id > NUM_TERMINALS || num_bytes > TERMINAL_MAX_LINE || num_bytes < 1 || tty_state->buffers[tty_id] == NULL || tty_id < 0) {
    return;
  }

  tty_state->bytes_available[tty_id] = num_bytes;
  if (tty_state->curr_readers[tty_id]->head == NULL) {
    // nobody is waiting to read the received bytes
    // set the bytes_available integer for tty_id to the number of bytes
    // that were just received
    return;
  }

  TracePrintf(1, "tty_receive: sending bytes to current readers\n");
  // otherwise, iterate through the list of processes waiting to read:
  // copy the information currently in the buffer at index tty_id into the pcb
  // tty reading buffer
  // unblock the pcb and mark it has having just received num_bytes from
  // the terminal
  int max_read = 0;
  set_node_t* curr = tty_state->curr_readers[tty_id]->head;
  while (curr != NULL) {
    if (curr->item == NULL) {
      curr = curr->next;
      continue;
    }

    pcb_t* proc = (pcb_t*)curr->item;
    if (proc->state == DEAD) {
      curr = curr->next;
      continue;
    }
    if (proc->tty_read_buffer_r0 == NULL) {
      proc->tty_read_buffer_r0 = malloc(TERMINAL_MAX_LINE * sizeof(char));
      if (proc->tty_read_buffer_r0 == NULL) {
        TracePrintf(1, "TTY Handle Received: Failed to mallloc!\n");
        return;
  }
    }
    
    TracePrintf(1, "tty_receive: Copying data into proc buffer with pid %d\n", proc->pid);
    // copy bytes into the pcb
    int num_bytes_to_copy = MIN(proc->tty_num_bytes_requested, num_bytes);
    memset(proc->tty_read_buffer_r0, 0, TERMINAL_MAX_LINE);
    memcpy(proc->tty_read_buffer_r0, tty_state->buffers[tty_id], num_bytes_to_copy);
    proc->tty_num_bytes_read = num_bytes_to_copy;
    max_read = MAX(max_read, num_bytes_to_copy);

    TracePrintf(1, "tty_receive: unblocking proc with pid %d\n", proc->pid);
    // unblock the pcb
    proc->state = READY;
    proc->tty_has_bytes = 1;
    TracePrintf(1, "tty_receive: popping pcb with pid %d from blocked processes \n", proc->pid);
    set_pop(blocked_pcbs, proc->pid);
    TracePrintf(1, "tty_receive: adding proc to ready queue \n", proc->pid);
    queue_push(process_ready_queue, proc);
    
    curr = curr->next;
  }

  TracePrintf(1, "tty_receive: All waiting processes have read bytes\n");
  // at this point all the processes have the information from the terminal input -> none are waiting
  tty_state->curr_readers[tty_id]->head = NULL;
  tty_state->curr_readers[tty_id]->node_count = 0;

  // consume the maximum number of bytes read by the reading processes
  tty_buffer_consume(tty_state, NULL, tty_id, max_read);
}

void
tty_buffer_consume(tty_state_t* tty_state, void* receipt_buffer, int tty_id, int num_bytes)
{
  if (tty_state == NULL || num_bytes < 1 || num_bytes > TERMINAL_MAX_LINE || tty_id > NUM_TERMINALS || tty_id < 0) {
    return;
  }

  if (receipt_buffer != NULL && !check_memory_validity(receipt_buffer)) {
    TracePrintf(1, "invalid buffer passed into tty_buffer_consume\n");
    return;
  }

  // check to make sure that the terminal actually has bytes available
  if (tty_state->bytes_available[tty_id] < 1) {
    return;
  }
  TracePrintf(1, "Consuming %d buffered bytes\n", num_bytes);

  // if the receipt buffer isn't null, copy the requested bytes into the buffer
  int num_bytes_to_copy = MIN(num_bytes, tty_state->bytes_available[tty_id]);
  if (receipt_buffer != NULL) {
    TracePrintf(1, "Copying remaining bytes into buffer\n", num_bytes);
    memset(receipt_buffer, 0, num_bytes);
    memcpy(receipt_buffer, tty_state->buffers[tty_id], num_bytes_to_copy);
  }
  
  // if all the bytes from the buffer were copied, set it to zero, and set bytes available to zero
  if (num_bytes_to_copy >= tty_state->bytes_available[tty_id]) {
    TracePrintf(1, "Reseting tty buffer\n", num_bytes);
    tty_state->bytes_available[tty_id] = 0;
    memset(tty_state->buffers[tty_id], 0, TERMINAL_MAX_LINE);
    return;
  }

  // if only some of them were copied, put the remaining ones in a temp
  // variable, set the buffer for tty_id to zeros, copy the temp back into
  // the start of the buffer, and set the bytes available to the difference
  // betwen the current bytes available and the number of copied bytes
  TracePrintf(1, "Left shifting buffer\n", num_bytes);
  int remaining_bytes = tty_state->bytes_available[tty_id] - num_bytes_to_copy;
  void* temp_byte_buffer = malloc(sizeof(char) * remaining_bytes);
  if (temp_byte_buffer == NULL) {
    TracePrintf(1, "TTY Buffer Consume: Failed to mallloc!\n");
    return;
  }
  memcpy(temp_byte_buffer, tty_state->buffers[tty_id] + num_bytes_to_copy, remaining_bytes);
  memset(tty_state->buffers[tty_id], 0, TERMINAL_MAX_LINE);
  memcpy(tty_state->buffers[tty_id], temp_byte_buffer, remaining_bytes);
  free(temp_byte_buffer);
  tty_state->bytes_available[tty_id] = remaining_bytes;
}

void
tty_state_free(tty_state_t* tty_state)
{
  // implement later
  return;
}
