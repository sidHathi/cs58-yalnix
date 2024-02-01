#include <yalnix.h>
#include <ykernel.h>
#include <yuser.h>
#include <hardware.h>
#include <util/queue.h>
#include <kernel.h>
#include <ykernel.h>
#include <load_info.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

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
  // frames at the top of the end of the page table (kernel stack) should also be valid and mapped to the same frame numbers in physical memory
  // the number and location stack frames can be calculated from KERNEL_STACK_BASE and KERNEL_STACK_LIMIT
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

KernelContext*
KCSwitch(KernelContext* kc_in, void* curr_pcb_p, void* next_pcb_p)
{
  // Check that the pointers passed in for each input are valid in memory and correspond to actual pcbs and kernel context
    // if this is not the case, return null

  // copy the bytes from kc_in into the curr_pcb_p's krn_ctx
  // copy the current region one page table into curr_pcb_p's page_table
  // store the contents of the CPU registers into the curr_pcb_p usr_ctx
  // Get the frame numbers for the kernel's stack using the region 0 page table ->
    // these will lie from KERNEL_STACK_BASE until the last (highest) page in virtual memory -> each page will reference its frame number
  // copy the frame numbers into curr_pcb_p
  // get the kernel stack frame numbers stored in next_pcb_p
  // use them to change the current region zero frame numbers for the stack pages to the ones stored in next_pcb_p
  // use the next_pcb's usr_ctx to populate the CPU registers and user stack
  // return the pointer to next_pcb_p's KernelContext stored in next_pcb_p->krn_ctx
}

KernelContext*
KCCopy(KernelContext* kc_in, void* new_pcb_p, void* not_used)
{
  // copy the kernel context kc_in into the new_pcb_p->krn_ctx
  // Get the frame numbers for the kernel's stack using the region 0 page table ->
    // these will lie from KERNEL_STACK_BASE until the last (highest) page in virtual memory -> each page will reference its frame number
  // copy them into new_pcb_p
  // return kc_in
}

void
ScheduleNextProcess()
{
  // if there are no processes on the ready queue -> return
  // otherwise, pop a pcb from the ready queue
  // call KernelContextSwitch with KCSwitch, the current pcb, and the popped pcb as its three parameters respectively
  // return
}

int
LoadProgram(char *name, char *args[], pcb_t* proc) 

{
  int fd;
  int (*entry)();
  struct load_info li;
  int i;
  char *cp;
  char **cpp;
  char *cp2;
  int argcount;
  int size;
  int text_pg1;
  int data_pg1;
  int data_npg;
  int stack_npg;
  long segment_size;
  char *argbuf;

  
  /*
   * Open the executable file 
   */
  if ((fd = open(name, O_RDONLY)) < 0) {
    TracePrintf(0, "LoadProgram: can't open file '%s'\n", name);
    return ERROR;
  }

  if (LoadInfo(fd, &li) != LI_NO_ERROR) {
    TracePrintf(0, "LoadProgram: '%s' not in Yalnix format\n", name);
    close(fd);
    return (-1);
  }

  if (li.entry < VMEM_1_BASE) {
    TracePrintf(0, "LoadProgram: '%s' not linked for Yalnix\n", name);
    close(fd);
    return ERROR;
  }

  /*
   * Figure out in what region 1 page the different program sections
   * start and end
   */
  text_pg1 = (li.t_vaddr - VMEM_1_BASE) >> PAGESHIFT;
  data_pg1 = (li.id_vaddr - VMEM_1_BASE) >> PAGESHIFT;
  data_npg = li.id_npg + li.ud_npg;
  /*
   *  Figure out how many bytes are needed to hold the arguments on
   *  the new stack that we are building.  Also count the number of
   *  arguments, to become the argc that the new "main" gets called with.
   */
  size = 0;
  for (i = 0; args[i] != NULL; i++) {
    TracePrintf(3, "counting arg %d = '%s'\n", i, args[i]);
    size += strlen(args[i]) + 1;
  }
  argcount = i;

  TracePrintf(2, "LoadProgram: argsize %d, argcount %d\n", size, argcount);
  
  /*
   *  The arguments will get copied starting at "cp", and the argv
   *  pointers to the arguments (and the argc value) will get built
   *  starting at "cpp".  The value for "cpp" is computed by subtracting
   *  off space for the number of arguments (plus 3, for the argc value,
   *  a NULL pointer terminating the argv pointers, and a NULL pointer
   *  terminating the envp pointers) times the size of each,
   *  and then rounding the value *down* to a double-word boundary.
   */
  cp = ((char *)VMEM_1_LIMIT) - size;

  cpp = (char **)
    (((int)cp - 
      ((argcount + 3 + POST_ARGV_NULL_SPACE) *sizeof (void *))) 
     & ~7);

  /*
   * Compute the new stack pointer, leaving INITIAL_STACK_FRAME_SIZE bytes
   * reserved above the stack pointer, before the arguments.
   */
  cp2 = (caddr_t)cpp - INITIAL_STACK_FRAME_SIZE;



  TracePrintf(1, "prog_size %d, text %d data %d bss %d pages\n",
	      li.t_npg + data_npg, li.t_npg, li.id_npg, li.ud_npg);


  /* 
   * Compute how many pages we need for the stack */
  stack_npg = (VMEM_1_LIMIT - DOWN_TO_PAGE(cp2)) >> PAGESHIFT;

  TracePrintf(1, "LoadProgram: heap_size %d, stack_size %d\n",
	      li.t_npg + data_npg, stack_npg);


  /* leave at least one page between heap and stack */
  if (stack_npg + data_pg1 + data_npg >= MAX_PT_LEN) {
    close(fd);
    return ERROR;
  }

  /*
   * This completes all the checks before we proceed to actually load
   * the new program.  From this point on, we are committed to either
   * loading succesfully or killing the process.
   */

  /*
   * Set the new stack pointer value in the process's UserContext
   */

  /* 
   * proc->usr_ctx.sp = cp2;
   */

  /*
   * Now save the arguments in a separate buffer in region 0, since
   * we are about to blow away all of region 1.
   */
  cp2 = argbuf = (char *)malloc(size);

  /* 
   * ==>> You should perhaps check that malloc returned valid space 
   */

  for (i = 0; args[i] != NULL; i++) {
    TracePrintf(3, "saving arg %d = '%s'\n", i, args[i]);
    strcpy(cp2, args[i]);
    cp2 += strlen(cp2) + 1;
  }

  /*
   * Set up the page tables for the process so that we can read the
   * program into memory.  Get the right number of physical pages
   * allocated, and set them all to writable.
   */

  /* ==>> Throw away the old region 1 virtual address space by
   * ==>> curent process by walking through the R1 page table and,
   * ==>> for every valid page, free the pfn and mark the page invalid.
   */

  /*
   * ==>> Then, build up the new region1.  
   * ==>> (See the LoadProgram diagram in the manual.)
   */

  /*
   * ==>> First, text. Allocate "li.t_npg" physical pages and map them starting at
   * ==>> the "text_pg1" page in region 1 address space.
   * ==>> These pages should be marked valid, with a protection of
   * ==>> (PROT_READ | PROT_WRITE).
   */

  /*
   * ==>> Then, data. Allocate "data_npg" physical pages and map them starting at
   * ==>> the  "data_pg1" in region 1 address space.
   * ==>> These pages should be marked valid, with a protection of
   * ==>> (PROT_READ | PROT_WRITE).
   */

  /* 
   * ==>> Then, stack. Allocate "stack_npg" physical pages and map them to the top
   * ==>> of the region 1 virtual address space.
   * ==>> These pages should be marked valid, with a
   * ==>> protection of (PROT_READ | PROT_WRITE).
   */

  /*
   * ==>> (Finally, make sure that there are no stale region1 mappings left in the TLB!)
   */

  /*
   * All pages for the new address space are now in the page table.  
   */

  /*
   * Read the text from the file into memory.
   */
  lseek(fd, li.t_faddr, SEEK_SET);
  segment_size = li.t_npg << PAGESHIFT;
  if (read(fd, (void *) li.t_vaddr, segment_size) != segment_size) {
    close(fd);
    return KILL;   // see ykernel.h
  }

  /*
   * Read the data from the file into memory.
   */
  lseek(fd, li.id_faddr, 0);
  segment_size = li.id_npg << PAGESHIFT;

  if (read(fd, (void *) li.id_vaddr, segment_size) != segment_size) {
    close(fd);
    return KILL;
  }


  close(fd);			/* we've read it all now */


  /*
   * ==>> Above, you mapped the text pages as writable, so this code could write
   * ==>> the new text there.
   *
   * ==>> But now, you need to change the protections so that the machine can execute
   * ==>> the text.
   *
   * ==>> For each text page in region1, change the protection to (PROT_READ | PROT_EXEC).
   * ==>> If any of these page table entries is also in the TLB, 
   * ==>> you will need to flush the old mapping. 
   */


  
  /*
   * Zero out the uninitialized data area
   */
  bzero(li.id_end, li.ud_end - li.id_end);

  /*
   * Set the entry point in the process's UserContext
   */

  /* 
   * ==>> (rewrite the line below to match your actual data structure) 
   * ==>> proc->uc.pc = (caddr_t) li.entry;
   */

  /*
   * Now, finally, build the argument list on the new stack.
   */


  memset(cpp, 0x00, VMEM_1_LIMIT - ((int) cpp));

  *cpp++ = (char *)argcount;		/* the first value at cpp is argc */
  cp2 = argbuf;
  for (i = 0; i < argcount; i++) {      /* copy each argument and set argv */
    *cpp++ = cp;
    strcpy(cp, cp2);
    cp += strlen(cp) + 1;
    cp2 += strlen(cp2) + 1;
  }
  free(argbuf);
  *cpp++ = NULL;			/* the last argv is a NULL pointer */
  *cpp++ = NULL;			/* a NULL pointer for an empty envp */

  return SUCCESS;
}
