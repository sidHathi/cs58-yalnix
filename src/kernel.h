#ifndef _kernel_h
#define _kenel_h

#include <yalnix.h>
#include <hardware.h>
#include "datastructures/queue.h"
#include "datastructures/pcb.h"

// Array of strings for kernel to store input from the terminals
extern char* tty_buffers[NUM_TERMINALS];

// global definition for free frame queue pointer -> actual queue lives in kernel heap
extern queue_t* free_frame_queue;

// global definitions for region 1 and 0 page tables -> stored in kernel data
extern pte_t region_0_pages[VMEM_REGION_SIZE/PAGESIZE];
extern pte_t region_1_pages[VMEM_REGION_SIZE/PAGESIZE];

// boolean that stores whether virtual memory is enabled
extern unsigned int virtual_mem_enabled = 0;

// number of bits that kernel break has been moved upwards from origin
extern unsigned long kernel_brk_offset = 0;

// queue that stores pointers to pcbs of ready processes
extern queue_t* process_ready_queue;

// array stores pointers to pcbs of blocked processes PCBs
extern pcb_t** process_blocked_arr;

// array that stores pointers to pcbs of dead processes
extern pcb_t** process_dead_arr;

// currently running process
extern pcb_t* current_process;

// Function passed into KernelContextSwitch to manage pcbs and kernel during process process switching
KernelContext* KCSwitch(KernelContext* kc_in, void* curr_pcb_p, void* next_pcb_p);

// Function passed into KernelContextSwitch to clone current kernel context and stack into new process
KernelContext* KCCopy(KernelContext* kc_in, void* new_pcb_p, void* not_used);

// Should be called at the end of every clock trap
void ScheduleNextProcess();

// load a new program into an existing address space
int LoadProgram(char *name, char *args[], pcb_t* proc);

#endif /*!_kernel_h*/
