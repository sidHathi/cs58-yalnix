#include <yuser.h>

#define MAX_INPUT 100

int
main(int argc, char** argv)
{
  char* input = malloc(MAX_INPUT * sizeof(char));
  TtyRead(0, input, MAX_INPUT);
  TtyPrintf(0, "input: %s\n", input);
}
