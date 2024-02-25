#ifndef _pcb_h
#define _pcb_h

#include <yalnix.h>
#include <hardware.h>
#include <ylib.h>
#include "linked_list.h"
#include "queue.h"

#define READY 0
#define BLOCKED 1
#define RUNNING 2
#define DEAD 3
#define NUM_KSTACK_FRAMES KERNEL_STACK_MAXSIZE/PAGESIZE

typedef struct pcb pcb_t;

typedef struct pcb {
	unsigned int state; // make this an enum with values for running, stopped, ready, blocked, etc
	unsigned int waiting; // stores whether process is waiting for children to terminate
	int pid;
	void* current_brk;
	int delay_ticks;
	int exit_status;
	int child_exit_status;
	pcb_t* parent;
	linked_list_t* children;
	linked_list_t* zombies;
	pte_t* page_table;
	UserContext* usr_ctx;
	KernelContext* krn_ctx; // stored on process switch
	pte_t* kernel_stack_pages;
	void* tty_read_buffer_r0; // region 0 addr
	void* tty_read_buffer_r1; // region 1 addr
	int tty_has_bytes; // boolean -> is there unconsumed terminal input in the pcb
	int tty_num_bytes_requested; // how many bytes of input is the process waiting for?
	int tty_num_bytes_read; // how many bytes were read into tty_read_buffer_r0
	int tty_write_waiting;
} pcb_t;

pcb_t* pcbNew(
  int pid, 
  pte_t* initial_page_table, 
  pte_t* initial_kstack_pages,
  pcb_t* parent, 
  UserContext* initial_user_ctx, 
  KernelContext* krn_ctx
);

// helper function for pcb list management:
// removes the pcb with pid `pid` from the `list`
// of pcb_t pointers
void
pcbListRemove(linked_list_t* list, int pid);

// frees the entire pcb
void pcbFree(pcb_t* pcb, queue_t* free_frame_queue);

// dumps any data that isn't needed after exits
void pcbExit(pcb_t* pcb, queue_t* free_frame_queue);

// makes child and zombies' parents init
void pcbOrphanChildren(pcb_t* pcb);

#endif /*!_pcb_h*/
