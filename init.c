#include <yuser.h>
#include <hardware.h>

int
main(int argc, char** argv)
{
  while (1) {
    TracePrintf(1, "Running init\n");
    Pause();
  }
}
