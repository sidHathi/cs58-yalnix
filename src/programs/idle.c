#include "datastructures/pcb.h"
#include <ylib.h>
#include <hardware.h>


void DoIdle(void) {
  while(1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}


// int create_idle_pcb() {
//   // pcbNew(int pid, pte_t* initial_page_table, pcb_t* parent, UserContext* initial_user_ctx)
//   pcb_t* idle_pcb = pcbNew()


//   return 0;
// }

