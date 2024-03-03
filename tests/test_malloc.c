#include <yuser.h>

int
main(int argc, char** argv)
{
  char* letsrunthisshit = malloc(10);
  TtyPrintf(0, "allocated memory addr 1: %p\n", letsrunthisshit);
  Delay(5);
  int childpid = Fork();
  if (childpid == 0) {
    char* childmem = malloc(100000000);
    TtyPrintf(0, "allocated memory addr 2: %p\n", childmem);
  }
  Wait(NULL);
}
