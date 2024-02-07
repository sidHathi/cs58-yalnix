#include <yalnix.h>
#include <ykernel.h>
#include <yuser.h>
#include <hardware.h>
#include "datastructures/queue.h"
#include "kernel.h"
#include <ykernel.h>
#include <load_info.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "traps.h"
#include "datastructures/pcb.h"
#include "datastructures/memory_cache.h"
#include "programs/idle.h"

// Macros that enumerate page table regions
#define REGION_UNUSED 0
#define REGION_BASE_MEMORY 1
#define REGION_KERNEL_TEXT 2
#define REGION_KERNEL_DATAHEAP 3
#define REGION_KERNEL_STACK 4

// Initialize globals
unsigned long kernel_brk_offset = 0;
unsigned int virtual_mem_enabled = 0;
unsigned int num_blocked_processes = 0;
unsigned int num_ready_processes = 0;
unsigned int num_dead_processes = 0;
char* tty_buffers[NUM_TERMINALS];
queue_t* free_frame_queue = NULL;
pte_t region_0_pages[VMEM_REGION_SIZE/PAGESIZE];
pte_t region_1_pages[VMEM_REGION_SIZE/PAGESIZE];
queue_t* process_ready_queue = NULL;
pcb_t** process_blocked_arr = NULL;
pcb_t** process_dead_arr = NULL;
pcb_t* current_process = NULL;

// local helper function prototyes
static void shrink_heap_pages(int prev_top_page_idx, int new_top_page_idx);
static void expand_heap_pages(int prev_top_page_idx, int new_top_page_idx);
static void init_page_tables();
static void init_free_frame_queue();

/**
 * Helper function: iterates over page from the old top index
 * to the new top index. checks to make sure each frame is not valid
 * if the page is valid -> makes it not valid. If virtual memory is
 * enabled, it adds each newly invalid frame to the free frames queue
 * If virtual memory is not enabled, it marks the frame as not being
 * used in the free_boot_frames array
*/
void
shrink_heap_pages(int prev_top_page_idx, int new_top_page_idx)
{
  for (int i = prev_top_page_idx; i > new_top_page_idx; i --) {
    if (!region_0_pages[i].valid) {
      continue;
    }
    region_0_pages[i].valid = 0;
    region_0_pages[i].prot = 0;

    if (virtual_mem_enabled) {
      int* page_loc = malloc(sizeof(int));
      *page_loc = region_0_pages[i].pfn;
      queuePush(free_frame_queue, page_loc);
    }
  }
}

/**
 * Helper function: In the case that virtual memory has not yet been 
 * enabled, all pages below the new brk must be mapped one to one 
 * with the frame number that equals the page index. In the case that
 * virtual memory is enabled, the brk should be raised -> idk if
*/
void
expand_heap_pages(int prev_top_page_idx, int new_top_page_idx)
{
  // loop from prev to new top page ++
  // find a frame for each new page from the free frame queue
  // But -> if virtual memory is not enabled -> take the frame
  // that has the page number
  // set the permissions for the page
  // mark as valid
  for (int i = prev_top_page_idx; i < new_top_page_idx; i ++) {
    int frame_no;
    if (virtual_mem_enabled) {
      frame_no = *(int*)queuePop(free_frame_queue);
    } else {
      frame_no = i; // figure out what to put here
    }

    region_0_pages[i].pfn = frame_no;
    region_0_pages[i].valid = 1;
    region_0_pages[i].prot = PROT_READ | PROT_WRITE;
  }
}

int
SetKernelBrk(void* addr)
{
  // IMPORTANT -> need to consider case where brk is shrinking
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

  int curr_brk_page_idx = _orig_kernel_brk_page + kernel_brk_offset;
  int suggested_brk_page_idx = UP_TO_PAGE(addr);
  // check to make sure that the suggested brk is not below the original brk
  // check to make sure that the suggested brk is not above the stack start page
  // check to make sure that if the brk is shrinking, there's nothing in the pages that will now be above the brk
  if (suggested_brk_page_idx < _orig_kernel_brk_page) {
    TracePrintf(1, "attempting to set kernel brk below original brk\n");
    return -1;
  }

  if (suggested_brk_page_idx > (int)DOWN_TO_PAGE((int)KERNEL_STACK_BASE)) {
    TracePrintf(1, "attempting to set kernel brk above kernel stack base\n");
    return -1;
  }

  if (suggested_brk_page_idx < curr_brk_page_idx) {
    shrink_heap_pages(curr_brk_page_idx, suggested_brk_page_idx);
  } else {
    expand_heap_pages(curr_brk_page_idx, suggested_brk_page_idx);
  }
  
  kernel_brk_offset = _orig_kernel_brk_page + suggested_brk_page_idx;
  return 0;
}

/**
 * Helper function -> sets validity and permissions for region 0 page table
*/
void
init_page_tables()
{
  // for region 0 ->
    // loop over frames
    // check frame region
    // if page is in kernel text ->
      // mark it as valid with exec and read permissions
    // if page is in kernel heap ->
      // mark it as valid with read and write permissions
    // if page is in kernel stack ->
      // mark it as valid with read and write permissions
    // otherwise ->
      // mark it as invalid with no permissions
  for (int page = 0; page < VMEM_REGION_SIZE/PAGESIZE; page ++) {
    int frame_region = REGION_UNUSED;
    // figure out where in memory the page is based on its numerical vlaue
    if (page < _first_kernel_text_page) {
      frame_region = REGION_BASE_MEMORY;
    } else if (page < _first_kernel_data_page) {
      frame_region = REGION_KERNEL_TEXT;
    } else if (page < _orig_kernel_brk_page + kernel_brk_offset) {
      frame_region = REGION_KERNEL_DATAHEAP;
    } else if (page >= KERNEL_STACK_BASE/PAGESIZE && page < KERNEL_STACK_LIMIT/PAGESIZE) {
      frame_region = REGION_KERNEL_STACK;
    }

    // set page table entry value based on memory region
    switch (frame_region) {
      case REGION_BASE_MEMORY:
        // pages below kernel text are not valid
        region_0_pages[page].valid = 0;
        region_0_pages[page].prot = 0;
        break;
      case REGION_KERNEL_TEXT:
        // pages in kernel text are executable and readable
        region_0_pages[page].pfn = page; // before virtual mem initialized -> pages and frame numbers must be equal
        region_0_pages[page].prot = PROT_EXEC | PROT_READ;
        region_0_pages[page].valid = 1;
        break;
      case REGION_KERNEL_DATAHEAP:
        // pages in the kernel data and heap are readable and writable
        region_0_pages[page].pfn = page;
        region_0_pages[page].prot = PROT_WRITE | PROT_READ;
        region_0_pages[page].valid = 1;
        break;
      case REGION_KERNEL_STACK:
        // pages in the kernel stack are readable and writeable
        region_0_pages[page].pfn = page;
        region_0_pages[page].prot = PROT_WRITE | PROT_READ;
        region_0_pages[page].valid = 1;
        break;
      default:
        region_0_pages[page].valid = 0;
        region_0_pages[page].prot = 0;
        break;
    }
  }

  // for region 1 ->
    // TEMPORARY:
    // mark all pages as invalid with no premissions
  for (int page = 0; page < VMEM_REGION_SIZE/PAGESIZE; page ++) {
    region_1_pages[page].valid = 0;
    region_1_pages[page].prot = 0;
  }
}

/**
 * Helper function -> places any frame number not used by the kernel
 * before memory it initialized into the free frame queue. This should
 * be run before virtual memory is initialized
*/
void
init_free_frame_queue()
{
  if (free_frame_queue == NULL) {
    TracePrintf(1, "attempting to use queue before initialization\n");
    return;
  }
  // iterate over all possible frame number
  // determine its region in memory before virtual memory was initialized
  // based on this region, infer whether it is already being used
  // in region 0 memory. If not -> add to free frame queue
  for (int frame = 0; frame < NUM_VPN; frame ++) {
    int frame_region = 0;
    if (frame < _first_kernel_text_page) {
      frame_region = 1;
    } else if (frame < _first_kernel_data_page) {
      frame_region = 2;
    } else if (frame < _orig_kernel_brk_page + kernel_brk_offset) {
      frame_region = 3;
    } else if (frame >= KERNEL_STACK_BASE/PAGESIZE && frame < KERNEL_STACK_LIMIT/PAGESIZE) {
      frame_region = 4;
    }

    if (frame_region == 0) {
      int* frame_no = malloc(sizeof(int));
      *frame_no = frame;
      queuePush(free_frame_queue, frame_no);
    }
  }
}

void
KernelStart(char** cmd_args, unsigned int pmem_size, UserContext* usr_ctx)
{
  // 1. allocate and initialize free frame queue
  // SetKernelBrk to some point far enough above origin to fit the entire queue
  // every frame above _first_kernel_data_page til max virtual pages should be added as a free frame -> only kernel text and below are being used
  virtual_mem_enabled = 0;
  free_frame_queue = queueCreate();
  
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
  init_page_tables();
  WriteRegister(REG_PTBR0, (unsigned int)region_0_pages);
  WriteRegister(REG_PTLR0, NUM_PAGES);
  WriteRegister(REG_PTBR1, (unsigned int)region_1_pages);
  WriteRegister(REG_PTLR1, NUM_PAGES);
  // 3.5: Add every frame that isn't mapped in the region 0 page table
  // to the free frames queue
  init_free_frame_queue();
  
  // 4. Enbale virtual memory by switching the register value in hardware + flush the TLB
  virtual_mem_enabled = 1;
  WriteRegister(REG_VM_ENABLE, 1);

  // 5. Allocate ready, blocked, and dead queues using queue_new functions
  process_ready_queue = queueCreate();
  num_ready_processes = 0;
  process_blocked_arr = malloc(sizeof(pcb_t*) * MAX_PROCESSES);
  num_blocked_processes = 0;
  process_dead_arr = malloc(sizeof(pcb_t*) * MAX_PROCESSES);
  num_dead_processes = 0;
  current_process = NULL;

  // 6. Allocate the interrupt vector and put the address in the appropriate register
  if (RegisterTrapHandlers() != 0) {
    TracePrintf(1, "trap handler registration failed\n");
  }
  else {
    TracePrintf(1, "Successfully registered trap handlers.\n");
  }

  // 7. Build Idle process PCB and make it current_process
  // To do: start the scheduler

  // TEMPORARY CHECKPOINT 2 SOLUTION TO LOAD IDLE PROGRAM
  int idle_stack_frame_1 = *(int*)queuePop(free_frame_queue);
  int stack_page_index = NUM_PAGES - 1;
  region_1_pages[stack_page_index].valid = 1;
  region_1_pages[stack_page_index].prot = PROT_READ | PROT_WRITE;
  region_1_pages[stack_page_index].pfn = idle_stack_frame_1;
  int idle_pid = helper_new_pid(region_1_pages);
  usr_ctx->pc = &DoIdle;
  usr_ctx->sp = (void*) (VMEM_1_LIMIT - 4);
  pcb_t* idle_pcb = pcbNew(idle_pid, region_1_pages, NULL, usr_ctx, NULL, NULL);
}

/*
// struct user_context {
//   int vector;		/* vector number */
//   int code;		/* additional "code" for vector */
//   void *addr;		/* offending address, if any */
//   void *pc;		/* PC at time of exception */
//   void *sp;		/* SP at time of exception */
//   void *ebp;              // base pointer at time of exception
//   u_long regs[GREGS];     /* general registers at time of exception */
// };
// */

unsigned int
check_memory_validity(void* pointer_addr)
{
  int byte_no = (int)pointer_addr;
  int page_addr = DOWN_TO_PAGE(byte_no);
  int page_no = page_addr/PAGESIZE;
  if (page_no > NUM_PAGES) {
    return -1;
  }
  pte_t page = region_0_pages[page_no];
  // maybe also check read/write permissions but not sure how to do yet
  if (page.valid) {
    return 1;
  }
  return 0;
}

KernelContext*
KCSwitch(KernelContext* kc_in, void* curr_pcb_p, void* next_pcb_p)
{
  // Check that the pointers passed in for each input are valid in memory and correspond to actual pcbs and kernel context
    // if this is not the case, return null
  if (!(check_memory_validity(curr_pcb_p) && check_memory_validity(next_pcb_p))) {
    return NULL;
  }

  pcb_t* curr_pcb = (pcb_t*)curr_pcb_p;
  pcb_t* next_pcb = (pcb_t*)next_pcb_p;
  if (!(check_memory_validity(curr_pcb->krn_ctx) && check_memory_validity(next_pcb->krn_ctx) && check_memory_validity(curr_pcb->kernel_stack_data) && check_memory_validity(next_pcb->kernel_stack_data))) {
    return NULL;
  }
  // copy the bytes from kc_in into the curr_pcb_p's krn_ctx
  memcpy(curr_pcb->krn_ctx, kc_in, sizeof(KernelContext));
  // copy the current region one page table into curr_pcb_p's page_table
  memcpy(curr_pcb->page_table, region_1_pages, NUM_PAGES*sizeof(pte_t));
  // store the contents of the CPU registers into the curr_pcb_p usr_ctx -> this needs to get done in the trap actually

  // Get the frame numbers for the kernel's stack using the region 0 page table ->
    // these will lie from KERNEL_STACK_BASE until the last (highest) page in virtual memory -> each page will reference its frame number
  void* kernel_stack_addr = (void*)KERNEL_STACK_BASE;
  memory_cache_t* kernel_cache = memory_cache_new(KERNEL_STACK_MAXSIZE/PAGESIZE, kernel_stack_addr);
  memory_cache_load(kernel_cache);
  curr_pcb->kernel_stack_data = kernel_cache;
  // get the kernel stack frame numbers stored in next_pcb_p
  // use them to change the current region zero frame numbers for the stack pages to the ones stored in next_pcb_p
  if (next_pcb->kernel_stack_data != NULL) {
    memory_cache_restore(next_pcb->kernel_stack_data);
    memory_cache_free(next_pcb->kernel_stack_data);
    next_pcb->kernel_stack_data = NULL;
  }
  // use the next_pcb's usr_ctx to populate the CPU registers and user stack -> again not sure this can happen yet
  // return the pointer to next_pcb_p's KernelContext stored in next_pcb_p->krn_ctx
  return next_pcb->krn_ctx;
}

KernelContext*
KCCopy(KernelContext* kc_in, void* new_pcb_p, void* not_used)
{
  // copy the kernel context kc_in into the new_pcb_p->krn_ctx
  // Get the frame numbers for the kernel's stack using the region 0 page table ->
    // these will lie from KERNEL_STACK_BASE until the last (highest) page in virtual memory -> each page will reference its frame number
  // copy them into new_pcb_p
  // return kc_in
  if (!check_memory_validity(new_pcb_p)) {
    return NULL;
  }
  pcb_t* new_pcb = (pcb_t*)new_pcb_p;
  if (!check_memory_validity(new_pcb->krn_ctx)) {
    return NULL;
  }

  memcpy(new_pcb->krn_ctx, kc_in, sizeof(KernelContext));
  if (new_pcb->kernel_stack_data != NULL && new_pcb->kernel_stack_data->cache_addr != NULL && check_memory_validity(new_pcb->kernel_stack_data->cache_addr)) {
    new_pcb->kernel_stack_data->original_addr = (void*)KERNEL_STACK_BASE;
    memory_cache_load(new_pcb->kernel_stack_data);
  } else {
    memory_cache_t* new_cache = memory_cache_new(KERNEL_STACK_MAXSIZE/PAGESIZE, (void*)KERNEL_STACK_BASE);
    memory_cache_load(new_cache);
    new_pcb->kernel_stack_data = new_cache;
  }

  return kc_in;
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
  cp2 = (void*)cpp - INITIAL_STACK_FRAME_SIZE;



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
  bzero((void*)li.id_end, li.ud_end - li.id_end);

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
