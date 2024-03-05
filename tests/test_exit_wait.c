#include <yuser.h>

int
main(int argc, char const *argv[])
{
  // Goal: test edge cases of exit and wait

  // test 1: wait without any children
  TracePrintf(1, "Running test exit wait\n");
  int status_ptr;
  TracePrintf(1, "calling wait without chlidren\n");
  int val = Wait(&status_ptr);
  assert(val == ERROR);
  TracePrintf(1, "calling wait with a null pointer and no children\n");
  val = Wait(NULL);
  assert(val == ERROR);
  // test 2: fork a process that exits and wait for it
  TracePrintf(1, "forking\n");
  int pid = Fork();
  if (pid == 0) {
    // runs in child
    TracePrintf(1, "running in forked child\n");
    Exit(0);
  }
  TracePrintf(1, "waiting for child\n");
  Wait(&status_ptr);
  TracePrintf(1, "Waited without crashing\n");
  // test 3: fork 2 processes and call wait 3 times
  int pid1 = Fork();
  int pid2 = Fork();
  if (pid1 == 0) {
    // runs in child
    Exit(0);
  } else if (pid2 == 0) {
    Exit(0);
  }
  Wait(&status_ptr);
  Wait(&status_ptr);
  int val3 = Wait(&status_ptr);
  assert(val3 == ERROR);
  TracePrintf(1, "waited for children successfuly\n");
  // test 5: fork a process that exits and call wait sometime later
  pid = Fork();
  if (pid == 0) {
    Exit(0);
  }
  Delay(5);
  int res = Wait(&status_ptr);
  assert(res == 0);
  TracePrintf(1, "waited successfuly after delay\n");
  // test 6: fork multiple processes that exit and call wait sometime later
  pid1 = Fork();
  pid2 = Fork();
  if (pid1 == 0 || pid2 == 0) {
    Delay(5);
    Exit(0);
  }
  res = Wait(&status_ptr);
  assert(res == 0);
  TracePrintf(1, "waited successfuly after block\n");
  // test 7: see if exit is handled successfuly for the init process 
  return 0;
}
