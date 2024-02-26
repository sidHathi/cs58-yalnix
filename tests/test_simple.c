#include <yuser.h>
#include <hardware.h>

int main(int argc, char* argv[]) {
  TracePrintf(1, "Welcome to a simple test!\n");
  int rc;
  Delay(3);
  TracePrintf(1, "Done delaying for 3 ticks\n");
  rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "I am the child! Here is my pid: %d\n", GetPid());
    Pause();
    Exit(0);
  }
  else {
    TracePrintf(1, "I am the parent. Here is my pid: %d\n", GetPid());
    Pause();
    Exit(0);
  }
}