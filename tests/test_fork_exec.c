#include <yuser.h>

int
main(int argc, char** argv)
{
  // tests for fork and exec
  // test 1: call exec with combinations of invalid parameters -> see how it reacts
  Exec(NULL, NULL);
  Exec("randompath", NULL);
  char* path = "randompath";
  Exec("randompath", &path);
  // test 2: call fork and exec in a normal scenario
  int pid = Fork();
  if (pid == 0) {
    Exec("init.c", NULL);
  }

  TracePrintf(1, "Yay, we made it past fork and exec. Now let's infinitely fork!\n");
  // test 3: infinitely call fork and see how the os handles the situation
  int i = 1;
  while (i < 5) {
    TracePrintf(1, "Fork #%d\n", i);
    pid = Fork();
    if (pid == 0) {
      return 0;
    }
    else {
      Wait(NULL);
      Delay(5);
    }
    i++;
  }
}
