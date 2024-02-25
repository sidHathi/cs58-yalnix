#include <yuser.h>

int main(int argc, char* argv[]) {
  TracePrintf(1, "Yurr pausing for dramatic effect\n");
  Pause();
  TracePrintf(1, "Here is my pid, just for fun: %d\n", GetPid());
  TracePrintf(1, "Delaying for 1 tick\n");
  Delay(1);
  TracePrintf(1, "Time to die\n");

  return 0;
}