#include <yuser.h>
#include <hardware.h>

int main(int argc, char* argv[]) {
  TracePrintf(1, "Welcome to a simple test!\n");
  TracePrintf(1, "About to fork...\n");
  int rc;
  rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "I am the child! Here is my pid: %d\n", GetPid());
    Exit(0);
  }
  else {
    TracePrintf(1, "I am the parent. Here is my pid: %d\n", GetPid());
    TracePrintf(1, "Exiting... Should go Idle\n");
    Exit(0);
  }
}