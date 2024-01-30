#include <yalnix.h>
#include <ykernel.h>
#include <yuser.h>
#include <hardware.h>
#include <util/queue.h>

int
SetKernelBrk(void* addr)
{
  // calculate offset from original brk
  // determine whether new address is valid
    // if not valid, return an error, traceprint
  // if virtual memory enabled:
    // calculate out how many new frames need to be enabled
    // for each one that doesn't have a frame, pop a random frame
    // from the global free frame queue and assign it to that page
    // set the appropriate permissions for the newly valid page
  // otherwise:
    // MANUALLY ASSIGN FRAME NUMBERS to the pages between current and new kernel brk
    // each new valid frame should map to the same frame number
    // in physical memory
    // if the queue contains that frame, remove it
  // modify the page table to reflect page-> frame mapping
  // set hardware register for kernel break
  // set global for kernel_brk_offset
  // return 0 if succesful
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
  // load page table address into appropriate register (REG_PTBR0)

  // 3. Set up page table for region 1
  // Involves looping to add each pte_t struct to the array
  // Each frame should have valid bit equal to zero and point to frame zero except one near the top of the address space for the user's stack
  // load page table address into appropriate register (REG_PTBR1)
  
  // 4. Enbale virtual memory by switching the register value in hardware + flush the TLB

  // 5. Allocate ready, blocked, and dead queues using queue_new functions

  // 6. Allocate the interrupt vector and put the address in the appropriate register

  // 7. Load the idle process and start the scheduler
}