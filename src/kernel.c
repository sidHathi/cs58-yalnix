#include "kernel.h"

// Macros that enumerate page table regions
#define REGION_UNUSED 0
#define REGION_BASE_MEMORY 1
#define REGION_KERNEL_TEXT 2
#define REGION_KERNEL_DATAHEAP 3
#define REGION_KERNEL_STACK 4


// Initialize globals
pcb_t* current_process = NULL;
pcb_t* init_process = NULL;
pcb_t* idle_process = NULL;
tty_state_t* current_tty_state = NULL;
queue_t* process_ready_queue;
set_t* delayed_pcbs = NULL;
set_t* dead_pcbs = NULL;
set_t* blocked_pcbs = NULL;
char* tty_buffers[NUM_TERMINALS];
queue_t* free_frame_queue;
pte_t region_0_pages[NUM_PAGES];
pte_t region_1_pages[NUM_PAGES];
unsigned int virtual_mem_enabled = 0;
unsigned long kernel_brk_offset = 0;
ipc_wrapper_t* ipc_wrapper = NULL;

// ****** IMPLEMENT HELPER FUNCTIONS *******

int
get_raw_page_no(void* addr)
{
  int page_no = (unsigned int)addr/PAGESIZE;
  return page_no;
}

/**
 * Helper function: iterates over page from the old top index
 * to the new top index. checks to make sure each frame is not valid
 * if the page is valid -> makes it not valid. If virtual memory is
 * enabled, it adds each newly invalid frame to the free frames queue
 * If virtual memory is not enabled, it marks the frame as not being
 * used in the free_boot_frames array
*/
int
shrink_heap_pages(int prev_top_page_idx, int new_top_page_idx)
{
  TracePrintf(1, "Moving kernelbrk from page %d to page %d\n", prev_top_page_idx, new_top_page_idx);
  for (int i = prev_top_page_idx; i > new_top_page_idx; i --) {
    if (!region_0_pages[i].valid) {
      continue;
    }
    region_0_pages[i].valid = 0;
    region_0_pages[i].prot = 0;

    if (virtual_mem_enabled) {
      int* page_loc = malloc(sizeof(int));
      if (page_loc == NULL) {
        return ERROR;
      }
      *page_loc = region_0_pages[i].pfn;
      queue_push(free_frame_queue, page_loc);
    }
  }
  return 0;
}

/**
 * Helper function: In the case that virtual memory has not yet been 
 * enabled, all pages below the new brk must be mapped one to one 
 * with the frame number that equals the page index. In the case that
 * virtual memory is enabled, the brk should be raised -> idk if
*/
int
expand_heap_pages(int prev_top_page_idx, int new_top_page_idx)
{
  // loop from prev to new top page ++
  // find a frame for each new page from the free frame queue
  // But -> if virtual memory is not enabled -> take the frame
  // that has the page number
  // set the permissions for the page
  // mark as valid
  TracePrintf(1, "Moving kernelbrk from page %d to page %d\n", prev_top_page_idx, new_top_page_idx);
  for (int i = prev_top_page_idx - 1; i < new_top_page_idx; i ++) {
    if (region_0_pages[i].valid) continue;

    int* frame_no_ptr;
    int frame_no;
    if (virtual_mem_enabled) {
      frame_no_ptr = (int*)queue_pop(free_frame_queue);
      if (frame_no_ptr == NULL) {
        return ERROR;
      }
      frame_no = *frame_no_ptr;
      free(frame_no_ptr);
    } else {
      frame_no = i; // figure out what to put here
    }

    region_0_pages[i].pfn = frame_no;
    region_0_pages[i].valid = 1;
    region_0_pages[i].prot = PROT_READ | PROT_WRITE;
  }
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
init_free_frame_queue(unsigned int num_frames)
{
  if (free_frame_queue == NULL) {
    TracePrintf(1, "attempting to use queue before initialization\n");
    return;
  }
  // iterate over all possible frame number
  // determine its region in memory before virtual memory was initialized
  // based on this region, infer whether it is already being used
  // in region 0 memory. If not -> add to free frame queue
  for (int frame = 0; frame < num_frames; frame ++) {
    int frame_region = 0;
    if (frame < _first_kernel_text_page) {
      frame_region = 1;
    } else if (frame < _first_kernel_data_page) {
      frame_region = 2;
    } else if (frame <= _orig_kernel_brk_page + kernel_brk_offset + 1) {
      // this check ensures that frames allocated to the kernel heap
      // before virtual memory initialization are not marked as free
      frame_region = 3;
    } else if (frame >= KERNEL_STACK_BASE/PAGESIZE && frame < KERNEL_STACK_LIMIT/PAGESIZE) {
      frame_region = 4;
    }

    if (frame_region == 0) {
      int* frame_no = malloc(sizeof(int));
      if (frame_no == NULL) {
        TracePrintf(1, "Init Free Frames: Failed to mallloc for the frame number\n");
        return;
      }
      *frame_no = frame;
      queue_push(free_frame_queue, frame_no);
    }
  }
}

// counts arguments passed into KernelStart
int
count_cmd_args(char** cmd_args)
{
  int i = 0;
  while (cmd_args[i] != NULL) {
    i ++;
  }
  return i;
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
  int suggested_brk_page_idx = UP_TO_PAGE(addr)/PAGESIZE;
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
    TracePrintf(1, "calling shrink_heap_pages\n");
    if (shrink_heap_pages(curr_brk_page_idx, suggested_brk_page_idx) == ERROR) {
      return ERROR;
    }
  } else {
    TracePrintf(1, "calling expand_heap_pages\n");
    if (expand_heap_pages(curr_brk_page_idx, suggested_brk_page_idx) == ERROR) {
      return ERROR;
    }
  }
  
  kernel_brk_offset = suggested_brk_page_idx - _orig_kernel_brk_page;
  return 0;
}

void
KernelStart(char** cmd_args, unsigned int pmem_size, UserContext* usr_ctx)
{
  // 1. allocate and initialize free frame queue
  // SetKernelBrk to some point far enough above origin to fit the entire queue
  // every frame above _first_kernel_data_page til max virtual pages should be added as a free frame -> only kernel text and below are being used
  virtual_mem_enabled = 0;
  
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
  free_frame_queue = queue_new();
  TracePrintf(1, "allocating queue for %d frames\n", pmem_size/PAGESIZE);
  init_free_frame_queue(pmem_size/PAGESIZE);
  
  // 4. Enbale virtual memory by switching the register value in hardware + flush the TLB
  virtual_mem_enabled = 1;
  WriteRegister(REG_VM_ENABLE, 1);

  // 5. Allocate ready, blocked, and dead sets using set_new functions
  process_ready_queue = queue_new();
  blocked_pcbs = set_new();
  dead_pcbs = set_new();
  current_process = NULL;

  // 6. Allocate the interrupt vector and put the address in the appropriate register
  if (RegisterTrapHandlers() != 0) {
    TracePrintf(1, "trap handler registration failed\n");
  }
  else {
    TracePrintf(1, "Successfully registered trap handlers.\n");
  }

  // Set up empty delay set
  delayed_pcbs = set_new();

  // Set up the idle pcb
  int* idle_sf1_pointer = (int*)queue_pop(free_frame_queue); // should switch this to a bit vector soon
  // for now -> check to make sure that the free frame queue returned something
  if (idle_sf1_pointer == NULL) {
    TracePrintf(1, "empty free frame queue\n");
    return;
  }
  int idle_stack_frame_1 = *idle_sf1_pointer;
  // free the data allocated to store the frame number on the queue
  free(idle_sf1_pointer);
  int stack_page_index = NUM_PAGES - 1;
  // NEW IN CHECKPOINT 3: allocate a new page table for the idle process
  pte_t* idle_pages = (pte_t*) malloc(sizeof(pte_t) * NUM_PAGES);
  if (idle_pages == NULL) {
        TracePrintf(1, "Kernel Start: Failed to mallloc idle pages!\n");
        return;
      }
  for (int i = 0; i < NUM_PAGES; i ++) {
    idle_pages[i].valid = 0; // all invalid to start
  }

  // set up idle user stack
  idle_pages[stack_page_index].valid = 1;
  idle_pages[stack_page_index].prot = PROT_READ | PROT_WRITE;
  idle_pages[stack_page_index].pfn = idle_stack_frame_1;
  int idle_pid = helper_new_pid(idle_pages);
  // set user context for idle and copy it into idle's pcb
  usr_ctx->pc = &DoIdle;
  usr_ctx->sp = (void*) (VMEM_1_LIMIT - 4);
  KernelContext* kernel_ctx = (KernelContext*)malloc(sizeof(KernelContext));
  if (kernel_ctx == NULL) {
        TracePrintf(1, "Kernel Start: Failed to mallloc for kernel context\n");
        return;
  }
  idle_process = pcbNew(idle_pid, idle_pages, NULL, NULL, usr_ctx, kernel_ctx);
  helper_check_heap("358");
  // set up idle kernel stack frames

  int* kernel_stack_1_p = (int*)queue_pop(free_frame_queue);
  int* kernel_stack_2_p = (int*)queue_pop(free_frame_queue);
  if (kernel_stack_1_p == NULL || kernel_stack_2_p == NULL) {
    TracePrintf(1, "No available frames for kernel stack\n");
  }
  idle_process->kernel_stack_pages[0].valid = 1;
  idle_process->kernel_stack_pages[0].prot = PROT_READ | PROT_WRITE;
  idle_process->kernel_stack_pages[0].pfn = *kernel_stack_1_p;
  idle_process->kernel_stack_pages[1].valid = 1;
  idle_process->kernel_stack_pages[1].prot = PROT_READ | PROT_WRITE;
  idle_process->kernel_stack_pages[1].pfn = *kernel_stack_2_p;
  TracePrintf(1, "Created idle pcb\n");
  free(kernel_stack_1_p);
  free(kernel_stack_2_p);

  // Set up data structure for IPC
  ipc_wrapper = ipc_wrapper_init();

  // parse command args to get the location of the init code -> if none provided then use default init.c
  int num_args = count_cmd_args(cmd_args);
  TracePrintf(1, "%d command args parsed in KernelStart\n", num_args);
  char* init_program_name = "init";
  if (num_args > 0) {
    init_program_name = cmd_args[0];
  }

  // Create the init pcb (using an empty page table, generated pid)
  pte_t* init_pages = malloc(sizeof(pte_t) * NUM_PAGES);
  if (init_pages == NULL) {
    return;
  }
  for (int i = 0; i < NUM_PAGES; i ++) {
    init_pages[i].valid = 0; // before LoadProgram, they're all invalid
  }

  // get pid for new proccess
  int init_pid = helper_new_pid(init_pages);
  init_process = pcbNew(init_pid, init_pages, NULL, NULL, usr_ctx, (KernelContext*)malloc(sizeof(KernelContext)));
  // Load the input program into the init pcb
  if (LoadProgram(init_program_name, cmd_args, init_process) != 0) {
    TracePrintf(1, "LoadProgram failed for init\n");
    Halt();
  }

  // initialize terminals
  current_tty_state = tty_state_init();
  
  // set the current process
  current_process = init_process;
  // Use KCCopy to copy the current kernel context into the new pcb
  KernelContextSwitch(&KCCopy, idle_process, NULL);
  TracePrintf(1, "KCCopy exited successfully\n");

  // Set the user stack and program counters to the ones stored 
  // in the current process' pcb before returning to userland
  memcpy(usr_ctx, current_process->usr_ctx, sizeof(UserContext));
}

unsigned int
check_memory_validity(void* pointer_addr)
{
  int region = 0;
  int byte_no = (int)pointer_addr;
  int page_addr = DOWN_TO_PAGE(byte_no);
  int page_no = page_addr/PAGESIZE;
  if (page_no > NUM_PAGES*2) {
    return -1;
  } else if (page_no > NUM_PAGES) {
    region = 1;
    page_no -= NUM_PAGES;
  }

  pte_t page = region_0_pages[page_no];
  if (region == 1) {
    pte_t* r1_pt = (pte_t*)ReadRegister(REG_PTBR1);
    page = r1_pt[page_no];
  }
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
  if ((!(curr_pcb_p == NULL || check_memory_validity(curr_pcb_p)) && (next_pcb_p == NULL || (next_pcb_p)))) {
    TracePrintf(1, "Memory invalid for KCSwitch pointer\n");
    return kc_in;
  }

  pcb_t* curr_pcb = (pcb_t*)curr_pcb_p;
  pcb_t* next_pcb = (pcb_t*)next_pcb_p;
  if (!check_memory_validity(next_pcb->krn_ctx)) {
    TracePrintf(1, "Memory invalid for KCSwitch kernel context pointer\n");
    return kc_in;
  }
  // copy the bytes from kc_in into the curr_pcb_p's krn_ctx
  if (curr_pcb != NULL && curr_pcb->krn_ctx != NULL && curr_pcb->kernel_stack_pages != NULL) {
    memcpy(curr_pcb->krn_ctx, kc_in, sizeof(KernelContext));

    for (int i = 0; i < (int)NUM_KSTACK_FRAMES; i ++) {
      curr_pcb->kernel_stack_pages[i] = region_0_pages[(NUM_PAGES - NUM_KSTACK_FRAMES) + i];
    }
  }

  for (int i = 0; i < (int)NUM_KSTACK_FRAMES; i ++) {
    region_0_pages[(NUM_PAGES - NUM_KSTACK_FRAMES) + i] = next_pcb->kernel_stack_pages[i];
  }
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

  current_process = next_pcb;

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
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  TracePrintf(1, "Running KCCopy\n");
  if (new_pcb_p == NULL || !check_memory_validity(new_pcb_p) || kc_in == NULL) {
    TracePrintf(1, "invalid pointer provided for new_pcb_p\n");
    return kc_in;
  }
  pcb_t* new_pcb = (pcb_t*)new_pcb_p;
  TracePrintf(1, "KCCopy memory check passed\n");

  if (new_pcb->krn_ctx == NULL) {
    new_pcb->krn_ctx = (KernelContext*) malloc(sizeof(KernelContext));
    if (new_pcb->krn_ctx == NULL) {
        TracePrintf(1, "Kernel Start: Failed to mallloc for kernel context in new_pcb\n");
        return NULL;
    }
  }
  TracePrintf(1, "kernel context pointer val %p, kc_in: %p\n", new_pcb->krn_ctx, kc_in);
  memcpy(new_pcb->krn_ctx, kc_in, sizeof(KernelContext));
  TracePrintf(1, "Copied kernel context into new pcb\n");

  // copy data into the pages
  // put the old pages back
  // clear cache again

  // set pges below kernel stack pages to new process' kernel stack pages
  for (int i = 0; i < (int)NUM_KSTACK_FRAMES; i ++) {
    region_0_pages[(int)(NUM_PAGES - 2*NUM_KSTACK_FRAMES) + i] = new_pcb->kernel_stack_pages[i];
  }
  TracePrintf(1, "allocated temporary pages for kernel frame copying\n");
  // clear tlb cache for region 0
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  // copy data into the pages
  void* temp_stack_addr = (void*)(KERNEL_STACK_BASE - KERNEL_STACK_MAXSIZE);
  TracePrintf(1, "copying from %p to %p\n", (void*)KERNEL_STACK_BASE, temp_stack_addr);
  memcpy(temp_stack_addr, (void*)KERNEL_STACK_BASE, KERNEL_STACK_MAXSIZE);
  helper_check_heap("copied data into new stack frames");
  for (int i = 0; i < (int)NUM_KSTACK_FRAMES; i ++) {
    region_0_pages[(int)(NUM_PAGES - 2*NUM_KSTACK_FRAMES) + i].valid = 0;
    region_0_pages[(int)(NUM_PAGES - 2*NUM_KSTACK_FRAMES) + i].prot = 0;
    region_0_pages[(int)(NUM_PAGES - 2*NUM_KSTACK_FRAMES) + i].pfn = 0;
  }
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  TracePrintf(2, "Exiting KCCopy\n");
  return kc_in;
}

void
enqueue_current_process()
{
  TracePrintf(1, "Enqueueing next process\n");
  if (current_process == NULL) {
    return;
  }

  switch (current_process->state) {
    case READY:
      TracePrintf(1, "adding process with pid %d to ready queue\n", current_process->pid);
      queue_push(process_ready_queue, current_process);
      break;
    case DEAD:
      TracePrintf(1, "adding process with pid %d to dead list\n", current_process->pid);
      set_insert(dead_pcbs, current_process->pid, current_process);
      break;
    case BLOCKED:
      TracePrintf(1, "adding process with pid %d to blocked list\n", current_process->pid);
      set_insert(blocked_pcbs, current_process->pid, current_process);
      break;
    case DELAYED:
      TracePrintf(1, "adding process with pid %d to delayed list\n", current_process->pid);
      set_insert(delayed_pcbs, current_process->pid, current_process);
      break;
    default:
      TracePrintf(1, "ERROR: Invalid process state for pid: %d\n", current_process->pid);
  }
}

void
ScheduleNextProcess()
{
  // if there are no processes on the ready queue -> return
  // otherwise, pop a pcb from the ready queue
  // call KernelContextSwitch with KCSwitch, the current pcb, and the popped pcb as its three parameters respectively
  // return
  
  // Checkpoint 3:
  // Move head of ready queue to current process and push current process to ready queue
  TracePrintf(1, "Entering scheduler \n");

  // Pop next process off ready queue
  pcb_t* next_process = (pcb_t*) queue_pop(process_ready_queue);

  // Handle empty ready queue
  if (next_process == NULL) {
    TracePrintf(1, "Scheduler: NEXT PROCESS NULL IN SCHEDULER\n");
    enqueue_current_process();
    next_process = (pcb_t*) queue_pop(process_ready_queue);
    // Switch to idle if no processes are ready to run
    if (next_process == NULL) {
      WriteRegister(REG_PTBR1, (unsigned int) idle_process->page_table);
      KernelContextSwitch(&KCSwitch, current_process, idle_process);
    }
    
  } else { // Round robin schedule
    TracePrintf(1, "Scheduler: Switching to process w/ pid %d from process w/ pid %d\n", next_process->pid, current_process != NULL ? current_process->pid : -1);
    enqueue_current_process();
    WriteRegister(REG_PTBR1, (unsigned int) next_process->page_table);
    KernelContextSwitch(&KCSwitch, current_process, next_process);
  }
  
  TracePrintf(1, "Leaving scheduler \n");
}

// HELPER -> Zeros out a page in virtual memory and frees the frame
// associated with that page by placing it on the free frame queue
void
free_page_frame(int region, int page_no)
{
  if (region > 1 || region < 0) {
    return;
  }
  // get the address of the page
  void* vmem_addr = (void*)((region * VMEM_REGION_SIZE) + page_no*PAGESIZE);
  if (!check_memory_validity(vmem_addr)) {
    return;
  }

  // zero out the memory
  memset(vmem_addr, 0, PAGESIZE);
  pte_t* page_table_addr = NULL;
  if (region == 0) {
    page_table_addr = region_0_pages;
  } else {
    page_table_addr = (void*) ReadRegister(REG_PTBR1);
  }

  // add the frame associated with that page to the free queue
  int frame_no = page_table_addr[page_no].pfn;
  page_table_addr[page_no].valid = 0;
  page_table_addr[page_no].prot = 0;
  int* stored_frame_no = (int*)malloc(sizeof(int));
  if (stored_frame_no == NULL) {
        TracePrintf(1, "Free Page Frame: Failed to mallloc for stored frame number\n");
        return;
  }
  *stored_frame_no = frame_no;
  queue_push(free_frame_queue, stored_frame_no);
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
   proc->usr_ctx->sp = cp2;

  /*
   * Now save the arguments in a separate buffer in region 0, since
   * we are about to blow away all of region 1.
   */
  cp2 = argbuf = (char *)malloc(size);
  /* 
   * ==>> You should perhaps check that malloc returned valid space 
   */
  if (argbuf == NULL || !check_memory_validity(argbuf)) {
    TracePrintf(2, "Malloc failed in LoadProgram\n");
    close(fd);
    return ERROR;
  }

  for (i = 0; args[i] != NULL; i++) {
    TracePrintf(3, "saving arg %d = '%s'\n", i, args[i]);
    strcpy(cp2, args[i]);
    cp2 += strlen(cp2) + 1;
  }

  /*
   * Set up the page tables for the process so that we can read the
   * program into memory. Get the right number of physical pages
   * allocated, and set them all to writable.
   */
  pte_t* new_process_pages = malloc(sizeof(pte_t)*NUM_PAGES);
  if (new_process_pages == NULL) {
        TracePrintf(1, "Load Program: Failed to mallloc new process pages!\n");
        return ERROR;
  }
  helper_check_heap("allocated new process pages");
  for (int i = 0; i < NUM_PAGES; i ++) {
    int region = 0;
    if (i > text_pg1 && i < data_pg1) {
      region = 1; // region 1 -> program text
    } else if (i < data_pg1 + data_npg) { // data_pg1 + data_npg is index of last page in heap
      region = 2;// region 1 -> program data
    } else if (i >= (NUM_PAGES - stack_npg)) {
      region = 4; // region 4 -> program stack
    }

    int* ff1_p = (int*)queue_pop(free_frame_queue);
    int* ff2_p = (int*)queue_pop(free_frame_queue);
    int* ff3_p = (int*)queue_pop(free_frame_queue);
    if (ff1_p == NULL || ff2_p == NULL || ff3_p == NULL) {
      TracePrintf(1, "No free frames found\n");
      return ERROR;
    }
    switch (region) {
      case 1:
        new_process_pages[i].valid = 1;
        new_process_pages[i].pfn = *ff1_p;
        new_process_pages[i].prot = PROT_READ | PROT_WRITE;
        break;
      case 2:
        new_process_pages[i].valid = 1;
        new_process_pages[i].pfn = *ff2_p;
        new_process_pages[i].prot = PROT_READ | PROT_WRITE;
        break;
        break;
      case 4:
        new_process_pages[i].valid = 1;
        new_process_pages[i].pfn = *ff3_p;
        new_process_pages[i].prot = PROT_READ | PROT_WRITE;
        break;
      default:
        new_process_pages[i].valid = 0;
        new_process_pages[i].prot = 0;
        break;
    }

    free(ff1_p);
    free(ff2_p);
    free(ff3_p);
  }

  // set brk for new process:
  // proc->brk = data_pg1 + data_npg + 1;
  proc->current_brk = (void*) ((data_pg1 + data_npg + 1)*PAGESIZE + VMEM_REGION_SIZE);

  /* ==>> Throw away the old region 1 virtual address space by
   * ==>> curent process by walking through the R1 page table and,
   * ==>> for every valid page, free the pfn and mark the page invalid.
   */
  pte_t* curr_region_1_pages = (pte_t*) ReadRegister(REG_PTBR1);
  for (int i = 0; i < NUM_PAGES; i ++) {
    if (!curr_region_1_pages[i].valid) {
      continue;
    }
    free_page_frame(1, i);
  }
  helper_check_heap("freed old region one");

  /*
   * ==>> Then, build up the new region1.  
   * ==>> (See the LoadProgram diagram in the manual.)
   */
  memcpy(proc->page_table, new_process_pages, NUM_PAGES * sizeof(pte_t));
  WriteRegister(REG_PTBR1, (unsigned int) proc->page_table);
  free(new_process_pages);
  helper_check_heap("copied page table into pcb");

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
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

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
  for (int i = text_pg1; i < data_pg1; i ++) {
    if (!proc->page_table[i].valid) {
      continue;
    }
    proc->page_table[i].prot = PROT_EXEC | PROT_READ;
  }
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

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
  proc->usr_ctx->pc = (void*) li.entry;

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