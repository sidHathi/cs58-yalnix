#ifndef _kernel_h
#define _kernel_h

#include <yalnix.h>
#include <hardware.h>
#include "datastructures/queue.h"
#include "datastructures/pcb.h"
#include "datastructures/set.h"


/**** MACROS ****/
#define MAX_PROCESSES 128 // Maximum number of processes allowed at any given time
#define NUM_PAGES VMEM_REGION_SIZE/PAGESIZE // Number of pages in a region


/*** GLOBAL VARIABLES ****/

// Specific PCB's
extern pcb_t* current_process;
extern pcb_t* init_process;
extern pcb_t* idle_process;

// Ready queue (data in each node is PCB of process that is ready to run) and number of nodes in this queue
extern queue_t* process_ready_queue;

// Sets mapping pid to PCB's, grouped by status
extern set_t* delayed_pcbs;
extern set_t* dead_pcbs;
extern set_t* blocked_pcbs;
// extern linked_list_t* delayed_pcb_list;
// extern linked_list_t* dead_pcb_list;
// extern linked_list_t* blocked_pcb_list;

// Array of strings for kernel to store input from the terminals
extern char* tty_buffers[NUM_TERMINALS];

// global definition for free frame queue pointer -> actual queue lives in kernel heap
extern queue_t* free_frame_queue;


// global definitions for region 1 and 0 page tables -> stored in kernel data
extern pte_t region_0_pages[NUM_PAGES];
extern pte_t region_1_pages[NUM_PAGES]; // this is only used by the idle process

// boolean that stores whether virtual memory is enabled
extern unsigned int virtual_mem_enabled;

// number of bits that kernel break has been moved upwards from origin
extern unsigned long kernel_brk_offset;




/**** IMPORTANT FUNCTIONS ****/

// Function passed into KernelContextSwitch to manage pcbs and kernel during process process switching
KernelContext* KCSwitch(KernelContext* kc_in, void* curr_pcb_p, void* next_pcb_p);

// Function passed into KernelContextSwitch to clone current kernel context and stack into new process
KernelContext* KCCopy(KernelContext* kc_in, void* new_pcb_p, void* not_used);

// Should be called at the end of every clock trap. Initializes a context switch.
void ScheduleNextProcess(UserContext* usr_ctx);

// Load a new program into an existing address space
int LoadProgram(char *name, char *args[], pcb_t* proc);

// Returns 1 if the virtual address passed belongs to a valid page in the current page table. Returns 0 otherwise.
unsigned int check_memory_validity(void* pointer_addr);

#endif /*!_kernel_h*/
