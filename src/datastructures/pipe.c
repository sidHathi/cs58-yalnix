#include <pipe.h>

typedef struct pipe_LL {
  pipe_t* head;
} pipe_LL_t;

typedef struct pipe {
  int id;
  queue_t* readers;
  queue_t* writers;
  pipe_t* next; // next p
  char* data;
} pipe_t;

pipe_t*
pipe_new(int id)
{
  // allocate memory for new empty pipe
  // return it
}

pipe_LL_t*
pipe_LL_new(pipe_t* head)
{
  // allocate memory for new pipe linked list
  // return it
}

void
pipe_free(pipe_t* pipe)
{
  // free memory allocated for pipe
}

void
pipe_LL_free(pipe_LL_t* pipe_LL)
{
  // free memory allocated for pipe linked list
}
