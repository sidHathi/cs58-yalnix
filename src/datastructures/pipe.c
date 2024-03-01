#include "pipe.h"

typedef struct {
	int pipe_id;
	void* buffer;
	int num_bytes_available;
	queue_t* readers;
	set_t* writers;
	int read_available;
	int write_available;
} pipe_t;

pipe_t*
pipe_new(int id)
{
  // allocate memory for new empty pipe
  // return it
  pipe_t* pipe = malloc(sizeof(pipe_t));

  pipe->pipe_id = id;
  pipe->buffer = malloc(sizeof(char) * PIPE_BUFFER_LEN);
  pipe->num_bytes_available = 0;
  pipe->readers = queue_new();
  pipe->writers = set_new();
  pipe->read_available = 1;
  pipe->write_available = 1;

  return pipe;
}

// helper function -> 
// shifts the bytes in the buffer left by the specified distance
int
leftshift_buffer(void* buffer, int buffer_size, int dist)
{
  void* tempbuffer = malloc(buffer_size - dist);
  memcpy(tempbuffer, buffer + dist, buffer_size - dist);
  memset(buffer, 0, buffer_size);
  memcpy(buffer, tempbuffer, buffer_size - dist);
}

// writes the given data into the pipe's buffer
// returns 0 on success, -1 on error, 1 if blocked
int
pipe_write(pipe_t* pipe, void* data, int len)
{
  // check if the data is from a valid address
  if (!check_memory_validity(data) || get_raw_page_no(data) < 128) {
    return -1;
  }
  // check if len is too large
  if (len > PIPE_BUFFER_LEN) {
    return -1;
  }
  // check if buffer is too full
  if (len + pipe->num_bytes_available > PIPE_BUFFER_LEN) {
    set_insert(pipe->writers, current_process->pid, current_process);
  }
  while (len + pipe->num_bytes_available > PIPE_BUFFER_LEN) {
    // block the current process
    current_process->state = BLOCKED;
    ScheduleNextProcess();
  }
  set_pop(pipe->writers, current_process->pid);

  // add the data to the buffer
  void* start_addr = pipe->buffer + pipe->num_bytes_available;
  memcpy(start_addr, data, len);
  // update the number of available bytes in the buffer
  pipe->num_bytes_available += len;

  // unblock next process waiting to read
  pcb_t* next_reader = queue_pop(pipe->readers);
  if (next_reader != NULL) {
    next_reader->state = READY;
    set_pop(blocked_pcbs, next_reader->pid);
    queue_push(process_ready_queue, next_reader);
  }
  return 0;
}

int
pipe_read(pipe_t* pipe, void* read_buf, int len)
{
  // check that the buffer is valid 
  if (!check_memory_validity(read_buf) || get_raw_page_no(read_buf) < 128) {
    return -1;
  }
  // check to see if there are bytes available
  while (pipe->num_bytes_available  < 1 && !pipe->read_available) {
    // block if none available
    current_process->state = BLOCKED;
    ScheduleNextProcess();
  }

  // at this point there should be bytes in the pipe to read
  // copy them into the buffer
  pipe->read_available = 0;
  int num_bytes_to_copy = MIN(len, pipe->num_bytes_available);
  memcpy(read_buf, pipe->buffer, num_bytes_to_copy);
  pipe->num_bytes_available -= num_bytes_to_copy;
  // leftshift the buffer
  leftshift_buffer(pipe->buffer, PIPE_BUFFER_LEN, num_bytes_to_copy);

  // if there are still bytes available, unblock next reader
  pcb_t* next_reader = queue_pop(pipe->readers);
  if (next_reader != NULL) {
    next_reader->state = READY;
    set_pop(blocked_pcbs, next_reader->pid);
    queue_push(process_ready_queue, next_reader);
  }

  // unblock any waiting writers
  for (set_node_t* pcb_node = pipe->writers->head; pcb_node != NULL; pcb_node = pcb_node->next) {
    pcb_t* pcb = pcb_node->item;
    if (pcb != NULL) {
      pcb->state = READY;
      set_pop(blocked_pcbs, pcb->pid);
      queue_push(process_ready_queue, pcb->state);
    }
  }

  return 0;
}

void
pipe_free(pipe_t* pipe)
{
  // free memory allocated for pipe
  free(pipe->buffer);
  queue_delete(pipe->readers, NULL);
  set_delete(pipe->writers, NULL);
  free(pipe);
}
