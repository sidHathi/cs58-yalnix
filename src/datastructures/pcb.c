#include <pcb.h>
#include <ylib.h>
#include <hardware.h>

typedef struct pcb {
	ulong state; // make this an enum with values for running, stopped, ready, blocked, etc
	int pid;
	pcb_t* parent;
	pcb_t** children;
	pte_t* page_table;
	UserContext* usr_ctx;
  KernelContext* krn_ctx; // stored on process switch
  ulong* kernel_stack_frames; // array of frame numbers
} pcb_t;

pcb_t*
pcbNew(int pid, pte_t* initial_page_table, pcb_t* parent, UserContext* initial_user_ctx)
{
  // allocate memory in kernel heap for the new pcb
  // set the default privelege to 0, and the default state to ready
  // use the parameters to the function to set the other fields
  // return the newly allocated pcb
  pcb_t * new_pcb = (pcb_t*) malloc(sizeof(pcb_t));
  new_pcb->state = READY;
  new_pcb->pid = pid;
  new_pcb->parent = parent;
  new_pcb->children = NULL;
  new_pcb->page_table = initial_page_table;
  new_pcb->usr_ctx = initial_user_ctx;
  new_pcb->krn_ctx = NULL;

  // Need to set this later, and figure out. NULL for now.
  new_pcb->kernel_stack_frames = NULL;

  return new_pcb;

}


// struct user_context {
//   int vector;		/* vector number */
//   int code;		/* additional "code" for vector */
//   void *addr;		/* offending address, if any */
//   void *pc;		/* PC at time of exception */
//   void *sp;		/* SP at time of exception */
//   void *ebp;              // base pointer at time of exception
//   u_long regs[GREGS];     /* general registers at time of exception */
// };


void
pcbFree(pcb_t* pcb)
{
  // free memory allocated for the pcb
}
