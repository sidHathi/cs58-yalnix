#ifndef _pcb_h
#define _pcb_h

#include <yalnix.h>
#include <hardware.h>
#include <ylib.h>
#include "memory_cache.h"

#define READY 0
#define BLOCKED 1
#define RUNNING 2
#define DEAD 3
#define NUM_KSTACK_FRAMES KERNEL_STACK_MAXSIZE/PAGESIZE

typedef struct pcb pcb_t;

typedef struct pcb {
	unsigned int state; // make this an enum with values for running, stopped, ready, blocked, etc
	int pid;
	pcb_t* parent;
	pcb_t** children;
	pte_t* page_table;
	UserContext* usr_ctx;
  KernelContext* krn_ctx; // stored on process switch
  memory_cache_t* kernel_stack_data; // array of frame numbers
	pte_t* kernel_stack_pages;
  int current_brk;
} pcb_t;

pcb_t* pcbNew(int pid, pte_t* initial_page_table, pcb_t* parent, UserContext* initial_user_ctx, KernelContext* krn_ctx);

void pcbFree(pcb_t* pcb);

#endif /*!_pcb_h*/
