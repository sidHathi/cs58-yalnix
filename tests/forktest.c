#include <yuser.h>



void
recurse(who, i)
     char *who;
     int i;
{
  TracePrintf(1, "running recurse with args %s, %d\n", who, i);
  TracePrintf(1, "wasting data in the stack %s, %d\n", who, i);
  char waste[1024];	/* use up stack space in the recursion */
  TracePrintf(1, "wasting data in the heap %s, %d\n", who, i);
  char *mem = (char *)malloc(2048); /* use up heap space */
  int j;

  TracePrintf(1, "accessing stack data %s, %d\n", who, i);
  for (j = 0; j < 1024; j++) 
    waste[j] = 'a';

  TtyPrintf(1, "%s %d\n", who, i);
  if (i == 0)
    {
      TtyPrintf(1, "Done with recursion\n");
      return;
    }
  else
    recurse(who, i - 1);
}


int main(argc, argv)
     int argc;
     char *argv[];
{
  int pid;

  TracePrintf(0,"BEFORE\n");

  pid = Fork();
  TracePrintf(1, "pid return value %d\n", pid);
  if (pid == 0)
    {
      TracePrintf(0,"CHILD\n");
      recurse("child", 33);
    }
  else
    {
      TracePrintf(0,"PARENT: child pid = %d\n", pid);
      recurse("parent", 33);
    }

  Exit(0);
}
