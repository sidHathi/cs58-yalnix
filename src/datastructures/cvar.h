#ifndef _cvar_h
#define _cvar_h

#include <yalnix.h>
#include <hardware.h>
#include "queue.h"

typedef struct cvar cvar_t;

cvar_t* cvar_new(int cvar_id, int owner);

void cvar_free(cvar_t* cvar);

#endif /*!_cvar_h*/
