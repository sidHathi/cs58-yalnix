#ifndef _kernel_h
#define _kernel_h

#include <yalnix.h>
#include <hardware.h>
#include "datastructures/queue.h"
#include "datastructures/linked_list.h"
#include "datastructures/pcb.h"

#define MAX_PROCESSES 128
#define NUM_PAGES VMEM_REGION_SIZE/PAGESIZE

extern linked_list_t* delayed_pcb_list;

// Array of strings for kernel to store input from the terminals
extern char* tty_buffers[NUM_TERMINALS];

// global definition for free frame queue pointer -> actual queue lives in kernel heap
extern queue_t* free_frame_queue;

// global definitions for a delayQueue, processes that are in this queue are to be blocked for the number of clock ticks
extern linked_list_t* delayed_pcb_list;

// global definitions for region 1 and 0 page tables -> stored in kernel data
extern pte_t region_0_pages[NUM_PAGES];
extern pte_t region_1_pages[NUM_PAGES]; // this is only used by the idle process

// boolean that stores whether virtual memory is enabled
extern unsigned int virtual_mem_enabled;

// number of bits that kernel break has been moved upwards from origin
extern unsigned long kernel_brk_offset;

// number of ready processes
extern unsigned int num_ready_processes;

// queue that stores pointers to pcbs of ready processes
extern queue_t* process_ready_queue;

// number of blocked processes
extern unsigned int num_blocked_processes;

// array stores pointers to pcbs of blocked processes PCBs
extern linked_list_t* blocked_pcb_list;

// number of dead processes
extern unsigned int num_dead_processes;

// array that stores pointers to pcbs of dead processes
extern linked_list_t* dead_pcb_list;

// currently running process
extern pcb_t* current_process;

// init process
extern pcb_t* init_process;

// Function passed into KernelContextSwitch to manage pcbs and kernel during process process switching
KernelContext* KCSwitch(KernelContext* kc_in, void* curr_pcb_p, void* next_pcb_p);

// Function passed into KernelContextSwitch to clone current kernel context and stack into new process
KernelContext* KCCopy(KernelContext* kc_in, void* new_pcb_p, void* not_used);

// Should be called at the end of every clock trap
void ScheduleNextProcess(UserContext* usr_ctx);

// load a new program into an existing address space
int LoadProgram(char *name, char *args[], pcb_t* proc);

#endif /*!_kernel_h*/
