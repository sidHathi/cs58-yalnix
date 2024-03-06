#include <yuser.h>

int main() {
  TracePrintf(1, "Welcome to cvar test\n");
  int lock_id, cvar_id, rc;
  LockInit(&lock_id);
  CvarInit(&cvar_id);
  TracePrintf(1, "%d %d YURR\n", lock_id, cvar_id);
  TracePrintf(1, "About to fork\n");
  rc = Fork();
  if (rc == 0) {
    Acquire(lock_id);
    TracePrintf(1, "I am the child and I have the lock! I can't be happy until my parent has had it!\n");
    int i = 1;
    while (i < 3) {
      TracePrintf(1, "Child spinning\n");
      CvarWait(cvar_id, lock_id);
      i++;
    }
    Release(lock_id);
    CvarSignal(cvar_id);
    Exit(0);
  }
  else {
    Delay(3);
    Acquire(lock_id);
    TracePrintf(1, "I am the parent and I have the lock!\n");
    Release(lock_id);
    CvarSignal(cvar_id);
    Wait(NULL);
  }
}
