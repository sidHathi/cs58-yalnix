#include <pcb.h>
#include <ylib.h>

typedef struct pcb {
  // 0 priv for user process, 1 for kernel
	ulong privelege; // is this a kernel or a user process?
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
pcb_new(int pid, pte_t* initial_page_table, pcb_t* parent, UserContext* initial_user_ctx)
{
  // allocate memory in kernel heap for the new pcb
  // set the default privelege to 0, and the default state to ready
  // use the parameters to the function to set the other fields
  // return the newly allocated pcb
}

void
pcb_free(pcb_t* pcb)
{
  // free memory allocated for the pcb
}
