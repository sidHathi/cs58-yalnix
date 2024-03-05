#include <yuser.h>

int main(int argc, char* argv[]) {
  TracePrintf(1, "Stüff\n");
  int lock_idp = 0;

  if (LockInit(&lock_idp) == ERROR) {
    TracePrintf(1, "Error creating lock :(\n");
    Exit(1);
  }

  int rc1 = Fork();
  if (rc1 == 0) {
    int rc2 = Fork();
    if (rc2 == 0) {
      TracePrintf(1, "Grandchild has pid %d. attempting to grab lock\n", GetPid());
      Acquire(lock_idp);
      TracePrintf(1, "I am the grandchild and I have the lock! Delaying for 5 ticks!\n");
      Delay(5);
      TracePrintf(1, "I am the grandchild and I still have the lock! Releasing!\n");
      Release(lock_idp);
      Exit(0);
    }
    else {
      TracePrintf(1, "Child has pid %d. attempting to grab lock\n", GetPid());
      Acquire(lock_idp);
      TracePrintf(1, "I am the child and I have the lock! Delaying for 5 ticks!\n");
      Delay(5);
      TracePrintf(1, "I am the child and I still have the lock! Releasing!\n");
      Release(lock_idp);
      Wait(NULL);
      Exit(0);
    }
  }
  else {
    TracePrintf(1, "Parent has pid %d. attempting to grab lock\n", GetPid());
    Acquire(lock_idp);
    TracePrintf(1, "I am the parent and I have the lock! Delaying for 5 ticks!\n");
    Delay(5);
    TracePrintf(1, "I am the parent and I still have the lock! Releasing!\n");
    Release(lock_idp);
    Wait(NULL);
    Exit(0);
  }
}