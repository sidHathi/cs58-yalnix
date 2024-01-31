#ifndef _kernel_h
#define _kenel_h

#include <yalnix.h>
#include <hardware.h>
#include <datastructures.h>

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
extern void** process_blocked_arr;

// array that stores pointers to pcbs of dead processes
extern void** process_dead_arr;

#endif /*!_kernel_h*/
