#include <yuser.h>
#include <hardware.h>

int main(int argc, char* argv[]) {
  TracePrintf(1, "Welcome to Asher and Brody's test!\n");
  TracePrintf(1, "About to fork...\n");
  int rc = Fork();
  rc = Fork();
  TracePrintf(1, "We just forked!\n");
  if (rc == 0) {
    Pause();
    TracePrintf(1, "I am the child! Here is my pid: %d\n", GetPid());
    return 0;
  }
  else {
    Pause();
    TracePrintf(1, "I am the parent. Here is my pid: %d", GetPid());
  }
  return 0;
}