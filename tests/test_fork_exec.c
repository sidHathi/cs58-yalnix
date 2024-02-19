#include <yuser.h>

int
main(int argc, char** argv)
{
  // tests for fork and exec
  // test 1: call exec with combinations of invalid parameters -> see how it reacts
  Exec(NULL, NULL);
  Exec("randompath", NULL);
  Exec("randompath", &"randompath");
  // test 2: call fork and exec in a normal scenario
  int pid = fork();
  if (pid == 0) {
    Exec("init.c", NULL);
  }
  // test 3: infinitely call fork and see how the os handles the situation
  while (1) {
    fork();
  }
}
