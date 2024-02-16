#include "pcb.h"
#include <ylib.h>
#include <hardware.h>

pcb_t*
pcbNew(
  int pid, 
  pte_t* initial_page_table, 
  pte_t* initial_kstack_pages,
  pcb_t* parent, 
  UserContext* initial_user_ctx, 
  KernelContext* krn_ctx
)
{
  // allocate memory in kernel heap for the new pcb
  // set the default privelege to 0, and the default state to ready
  // use the parameters to the function to set the other fields
  // return the newly allocated pcb
  pcb_t * new_pcb = (pcb_t*) malloc(sizeof(pcb_t));
  new_pcb->state = READY;
  new_pcb->pid = pid;
  new_pcb->delay_ticks = 0;
  new_pcb->children = linked_list_create();
  new_pcb->zombies = linked_list_create();
  new_pcb->parent = parent;
  new_pcb->children = NULL;
  new_pcb->page_table = initial_page_table;
  new_pcb->usr_ctx = (UserContext*) malloc(sizeof(UserContext));
  memcpy(new_pcb->usr_ctx, initial_user_ctx, sizeof(UserContext));
  if (krn_ctx == NULL) {
    new_pcb->krn_ctx = (KernelContext*) (sizeof(KernelContext));
  } else {
    new_pcb->krn_ctx = krn_ctx;
  }
  if (initial_kstack_pages == NULL) {
    new_pcb->kernel_stack_pages = malloc(sizeof(pte_t) * NUM_KSTACK_FRAMES);
  } else {
    new_pcb->kernel_stack_pages = initial_kstack_pages;
  }

  return new_pcb;
}

void
pcbFree(pcb_t* pcb)
{
  // free memory allocated for the pcb
  // should not free parents or children -> but the linked list used to store them should be removed
  // page table should be freed
  if (pcb == NULL) {
    return;
  }
  if (pcb->page_table != NULL) {
    free(pcb->page_table);
    pcb->page_table = NULL;
  }
  if (pcb->children != NULL) {
    linked_list_free(pcb->children, NULL);
    pcb->children = NULL;
  }
  if (pcb->zombies != NULL) {
    linked_list_free(pcb->zombies, NULL);
    pcb->zombies = NULL;
  }
  if (pcb->usr_ctx != NULL) {
    free(pcb->usr_ctx);
    pcb->usr_ctx = NULL;
  }
  if (pcb->krn_ctx != NULL) {
    free(pcb->krn_ctx);
    pcb->krn_ctx = NULL;
  }
  if (pcb->kernel_stack_pages != NULL) {
    free(pcb->kernel_stack_pages);
    pcb->kernel_stack_pages = NULL;
  }
  free(pcb);
}
