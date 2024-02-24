#include <yuser.h>

int main(int argc, char* argv[]) {
  TracePrintf(1, "Init pausing for dramatic effect\n");
  Pause();
  TracePrintf(1, "Time to die\n");
  return 0;
}