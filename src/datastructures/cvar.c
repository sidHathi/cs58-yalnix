#include <cvar.h>
#include <queue.h>

typedef struct cvar {
  int cvar_id; // unique identifier
  int owner; // pid of process using cvar
  queue_t* blocked; // FIFO queue of PCB's waiting on cvar
} cvar_t;

cvar_t*
cvar_new(int cvar_id, int owner)
{
  // allocate memory for new cvar with an empty blocked queue
  // return it
}

void
cvar_free(cvar_t* cvar)
{
  // free memory allocated for the cvar
}
