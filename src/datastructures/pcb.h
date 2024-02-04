#ifndef _pcb_h
#define _pcb_h

#include <yalnix.h>
#include <hardware.h>

#define READY 0
#define BLOCKED 1
#define RUNNING 2
#define DEAD 3

typedef struct pcb pcb_t;

pcb_t* pcbNew(int pid, pte_t* initial_page_table, pcb_t* parent, UserContext* initial_user_ctx);

void pcb_free(pcb_t* pcb);

#endif /*!_pcb_h*/
