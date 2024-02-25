#include "queue.h"
#include "linked_list.h"
#include "pcb.h"

typedef struct tty_state {
  queue_t** write_queues; // array of NUM_TERINALS queues for processes waiting to write
  int* availability; // array of NUM_TERMINALS 0/1 values 
  pcb_t** curr_writers; // array of NUM_TERINALS processes currently writing to a given terminal - NULL if none 
  linked_list_t** curr_readers; // array of NUM_TERINALS linked lists of processes currently reading from a given terminal
  void** buffers; // NUM_TERMINALS character buffers for each indexed terminal
  int* bytes_available; // number of bytes available to read in each terminal
} tty_state_t;

tty_state_t* tty_state_init();

// called in TRAP_TTY_RECIVE when terminal tty_id received new bytes
// if the curr_readers list for the given terminal is empty, the handler 
// should mark the corresponding buffer as having num_bytes bytes available
// and return
// otherwise, it should copy the received line into the pcb buffers
// for each waiting process and mark the line as having been consumed
// up to the maximum requested length from any of the processes
void tty_handle_received(tty_state_t* tty_state, int tty_id, int num_bytes);

// Called when a process consumes data that has already been received from a terminal
// if no bytes are available for the terminal at tty_id, returns immediately
// Clears the buffer at index tty_id up to the specified length
// copies the requested number of bytes into the receipt_buffer
// sets bytes available to zero for index tty_id
void tty_buffer_consume(tty_state_t* tty_state, void* receipt_buffer, int tty_id, int num_bytes);

void tty_state_free(tty_state_t* tty_state);
