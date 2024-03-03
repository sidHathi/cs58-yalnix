#include <yuser.h>
#include <yalnix.h>

int
main(int argc, char** argv)
{
  int pipeidp;
  PipeInit(&pipeidp);

  TtyPrintf(0, "new pipe initialized with idp %d\n", pipeidp);
  PipeWrite(pipeidp, "piped data", strlen("piped data"));

  char* buf = malloc(strlen("piped data"));
  PipeRead(pipeidp, buf, strlen("piped data"));

  TtyPrintf(0, "Read %s from pipe\n", buf);
  
  int childpid = Fork();
  if (childpid == 0) {
    TtyPrintf(0, "Child attempting to read from (hopefully) empty pipe\n");
    char* buffer = malloc(sizeof(char) * 100);
    PipeRead(pipeidp, buffer, 20);
    Delay(2);
    TtyPrintf(0, "Child read %s from the pipe after delay\n", buffer);
    Exit(0);
  }

  int childpid2 = Fork();
  if (childpid2 == 0) {
    TtyPrintf(0, "Second child attempting to read from (hopefully) empty pipe\n");
    char* buffer = malloc(sizeof(char) * 100);
    PipeRead(pipeidp, buffer, strlen("second message from parent"));
    Delay(2);
    TtyPrintf(0, "Child read %s from the pipe after delay\n", buffer);
    Exit(0);
  }
  
  Delay(3);
  TtyPrintf(0, "parent writing first message\n");
  PipeWrite(pipeidp, "message from parent", strlen("message from parent"));
  Delay(3);
  TtyPrintf(0, "parent writing second message\n");
  PipeWrite(pipeidp, "second message from parent", strlen("second message from parent"));
  Wait(NULL);
  Wait(NULL);

  // test overfilling the pipe
  char* bigBuffer = malloc(PIPE_BUFFER_LEN - 1);
  memset(bigBuffer, 'x', PIPE_BUFFER_LEN - 1);
  PipeWrite(pipeidp, bigBuffer, PIPE_BUFFER_LEN - 1);

  int childpid3 = Fork();
  if (childpid3 == 0) {
    // attempt to write to the pipe
    TtyPrintf(0, "Child 3 is attemptint to write to a full pipe\n");
    PipeWrite(pipeidp, "overflow message", strlen("overflow message"));
    TtyPrintf(0, "Child 3 was able to write successfuly\n");
    Exit(0);
  }

  TtyPrintf(0, "parent is delaying\n");
  Delay(5);
  TtyPrintf(0, "parent will now read from it's own pipe\n");
  PipeRead(pipeidp, bigBuffer, PIPE_BUFFER_LEN - 1);

  Wait(NULL);

  Reclaim(pipeidp);
  TtyPrintf(0, "parent exits\n");
}
