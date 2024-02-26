#include <yuser.h>

#define MAX_INPUT 100

int
main(int argc, char** argv)
{
  char* input = malloc(MAX_INPUT * sizeof(char));
  TtyRead(0, input, MAX_INPUT);
  TtyPrintf(0, "input: %s\n", input);

  TtyPrintf(0, "another line\n");
  TtyRead(0, input, MAX_INPUT);
  TtyRead(0, input, MAX_INPUT);
  TtyPrintf(0, "input: %s\n", input);

  TtyPrintf(2, "Let's put something on terminal two\n");
  TtyRead(2, input, MAX_INPUT);
  TtyPrintf(1, "Got something on terminal 2\n");
}
