#ifndef _pcb_h
#define _pcb_h

#include <yalnix.h>
#include <hardware.h>

typedef struct pcb pcb_t;

pcb_t* pcb_new(int pid, pte_t* initial_page_table, pcb_t* parent, UserContext* initial_user_ctx);

void pcb_free(pcb_t* pcb);

#endif /*!_pcb_h*/
