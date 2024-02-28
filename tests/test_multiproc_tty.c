#include <yuser.h>

// testing program -> 
// goal is to write and read from the same terminals at the same time
// should Fork off a few processes and have them execute write and read
// instructions to the terminals

int
main(int argc, char** argv)
{
  int pid1 = Fork();
  int pid2 = -1;
  int pid3 = -1;
  if (pid1 != 0) {
    pid2 = Fork();
  }
  if (pid1 != 0 && pid2 != 0) {
    pid3 = Fork();
  }
  if (pid1 == 0) {
    char* input = malloc(sizeof(char) * 100);
    TtyRead(0, input, 100);
    TtyPrintf(1, "child 1 read: %s\n", input);
    for (int i = 0; i < 10; i ++) {
      TtyPrintf(0, "Child 1 printing to terminal\n");
      Delay(1);
    }
    Exit(0);
  } else if (pid2 == 0) {
    char* input = malloc(sizeof(char) * 100);
    TtyRead(0, input, 100);
    TtyPrintf(2, "child 2 read: %s\n", input);
    for (int i = 0; i < 10; i ++) {
      TtyPrintf(0, "Child 2 printing to terminal\n");
      Delay(1);
    }
    Exit(0);
  } else if (pid3 == 0) {
    char* input = malloc(sizeof(char) * 100);
    TtyRead(0, input, 100);
    TtyPrintf(3, "child 3 read: %s\n", input);
    for (int i = 0; i < 10; i ++) {
      TtyPrintf(0, "Child 3 printing to terminal\n");
      Delay(1);
    }
    Exit(0);
  } else {
    Delay(10);
    for (int i = 0; i < 10; i ++) {
      TtyPrintf(0, "Parent printing to terminal\n");
      Delay(1);
    }
    int status_pointer;
    Wait(&status_pointer);
    Wait(&status_pointer);
    Wait(&status_pointer);
    TtyPrintf(0, "Success!\n");
    Exit(0);
  }
}
