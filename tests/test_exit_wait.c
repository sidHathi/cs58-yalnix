#include <yuser.h>

int
main(int argc, char const *argv[])
{
  // Goal: test edge cases of exit and wait

  // test 1: wait for process that doesn't exist
  // test 2: fork a process that exits and wait for it
  // test 3: fork 4 processes and call wait 5 times
  // test 5: fork a process that exits and call wait sometime later
  // test 6: fork multiple processes that exit and call wait sometime later
  // test 7: see if exit is handled successfuly for the init process 
  return 0;
}
