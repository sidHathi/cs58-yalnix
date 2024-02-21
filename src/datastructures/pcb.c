#include "pcb.h"
#include <ylib.h>
#include <hardware.h>
#include "kernel.h"

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
  
  if(new_pcb == NULL) {
    TracePrintf(1, "newPCB: failed to malloc new pcb\n");
    return NULL;
  }
  new_pcb->state = READY;
  new_pcb->pid = pid;
  new_pcb->delay_ticks = 0;
  new_pcb->children = linked_list_create();
  new_pcb->zombies = linked_list_create();
  new_pcb->parent = parent;
  new_pcb->page_table = initial_page_table;
  new_pcb->usr_ctx = (UserContext*) malloc(sizeof(UserContext));
  new_pcb->exit_status = 0;
  new_pcb->child_exit_status = 0;
  new_pcb->waiting = 0;
  memcpy(new_pcb->usr_ctx, initial_user_ctx, sizeof(UserContext));
  if (krn_ctx == NULL) {
    new_pcb->krn_ctx = (KernelContext*) malloc(sizeof(KernelContext));
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
pcbOrphanChildren(pcb_t* pcb)
{
  if (pcb->children != NULL) {
    lnode_t* curr = pcb->children->front;
    while (curr != NULL) {
      pcb_t* child = (pcb_t*)curr->data;
      child->parent = init_process;

      curr = curr->next;
    }
  }

  if (pcb->zombies != NULL) {
    lnode_t* curr = pcb->zombies->front;
    while (curr != NULL) {
      pcb_t* zombie = (pcb_t*)curr->data;
      zombie->parent = init_process;

      curr = curr->next;
    }
  }
}

void
pcbFree(pcb_t* pcb, queue_t* free_frame_queue)
{
  // free memory allocated for the pcb
  // should not free parents or children -> but the linked list used to store them should be removed
  // page table should be freed
  if (pcb == NULL) {
    return;
  }
  if (pcb->page_table != NULL) {
    if (free_frame_queue != NULL) {
      for (int i = 0; i < NUM_PAGES; i ++) {
        pte_t page_entry = pcb->page_table[i];
        if (page_entry.valid) {
          int* pfn_holder = (int*) malloc(sizeof(int));
          *pfn_holder = page_entry.pfn;
          queuePush(free_frame_queue, pfn_holder);
        }
      }
    }
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

void
pcbExit(pcb_t* pcb, queue_t* free_frame_queue)
{
   if (pcb == NULL) {
    return;
  }
  if (pcb->page_table != NULL) {
    if (free_frame_queue != NULL) {
      for (int i = 0; i < NUM_PAGES; i ++) {
        pte_t page_entry = pcb->page_table[i];
        if (page_entry.valid) {
          int* pfn_holder = (int*) malloc(sizeof(int));
          *pfn_holder = page_entry.pfn;
          queuePush(free_frame_queue, pfn_holder);
        }
      }
    }
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
}
