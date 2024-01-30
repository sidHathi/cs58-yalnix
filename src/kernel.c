#include <yalnix.h>
#include <ykernel.h>
#include <yuser.h>
#include <hardware.h>
#include <util/queue.h>

// global definition for free frame queue pointer -> actual queue lives in kernel heap
queue_t* free_frame_queue;

// global definitions for region 1 and 0 page tables -> stored in kernel data
pte_t region_0_pages[NUM_VPN];
pte_t region_1_pages[NUM_VPN];

// boolean that stores whether virtual memory is enabled
unsigned int virtual_mem_enabled = 0;

// number of bits that kernel break has been moved upwards from origin
unsigned long kernel_brk_offset = 0;

// queue that stores pointers to pcbs of ready processes
queue_t* process_ready_queue;

// queue htat stores pointers to pcbs of blocked processes
queue_t* process_blocked_queue;

// queue htat stores pointers to pcbs of blocked processes
queue_t* dead_queue;

int
SetKernelBrk(void* addr)
{
  // calculate offset from original brk
  // determine whether new address is valid
  // set hardware register for kernel break
  // set global for kernel_brk_offset
}

void
KernelStart(const char** cmd_args, unsigned int pmem_size, UserContext* usr_ctx)
{
  // 1. allocate and initialize free frame queue
  // SetKernelBrk to some point far enough above origin to fit the entire queue
  // every frame above _first_kernel_data_page til max virtual pages should be added as a free frame -> only kernel text and below are being used
  
  // 2. Set up page table for region 0
  // Involves looping to add each pte_t struct to the array
  // Every frame up til and including the kernel heap (up to kernelbrk) should
  // be valid and mapped to the same frame number as page number
  // Frames from _first_kernel_text_page to _first_kernel_data_page should have
  // restricted read and write permissions
  // Frames below _first_kernel_text_page should have restricted write permissions
  // the rest of the pages should not be valid for this region

  // 3. Set up page table for region 1
  // Involves looping to add each pte_t struct to the array
  // Each frame should have valid bit equal to zero and point to frame zero except one near the top of the address space for the user's stack
  
  // 4. Enbale virtual memory by switching the register value in hardware

  // 5. Allocate ready, blocked, and dead queues using queue_new functions

  // 6. Load the idle process and start the scheduler
}