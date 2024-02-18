#include <yuser.h>

int
main(int argc, char const *argv[])
{
  // Goal: test edge cases of exit and wait

  // test 1: wait without any children
  int status_ptr;
  int val = Wait(&status_ptr);
  assert(val == ERROR);
  val = Wait(NULL);
  assert(val == ERROR);
  // test 2: fork a process that exits and wait for it
  int pid = fork();
  if (pid == 0) {
    // runs in child
    Exit(0);
  }
  Wait(&status_ptr);
  // test 3: fork 2 processes and call wait 3 times
  int pid1 = fork();
  int pid2 = fork();
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
  // test 5: fork a process that exits and call wait sometime later
  pid = fork();
  if (pid == 0) {
    Exit(0);
  }
  Delay(5);
  int res = Wait(&status_ptr);
  assert(res == 0);
  // test 6: fork multiple processes that exit and call wait sometime later
  pid1 = fork();
  pid2 = fork();
  if (pid == 0) {
    Exit(0);
  }
  Delay(5);
  int res = Wait(&status_ptr);
  assert(res == 0);
  // test 7: see if exit is handled successfuly for the init process 
  return 0;
}
