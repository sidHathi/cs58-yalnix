#include <yuser.h>


typedef struct my_struct {
  int cookies;
  int lock_id;
  int cvar_id;
} my_struct_t;

int main(int argc, char* argv[]) {
  my_struct_t* cookie_jar = (my_struct_t*) malloc(sizeof(my_struct_t));
  cookie_jar->cookies = 0;
  LockInit(&cookie_jar->lock_id);
  if (CvarInit(&cookie_jar->cvar_id) == ERROR) {
    TracePrintf(1, "Couldn't create cvar :( \n");
    Exit(-1);
  }

  TracePrintf(1, "Initially, there are %d cookies in the cookie jar.\n", cookie_jar->cookies);

  int rc = Fork();

  if (rc == 0) {
    TracePrintf(1, "I am the child.\n");
    Pause();
    while (Acquire(cookie_jar->lock_id) == 0) {
      TracePrintf(1, "Child has the lock\n");
      if (cookie_jar->cookies < 5) {
        TracePrintf(1, "Child got the lock but there are only %d cookies!\n", cookie_jar->cookies);
        Pause();
        CvarWait(cookie_jar->cvar_id, cookie_jar->lock_id);
      }
      else {
        break;
      }
    }
    TracePrintf(1, "Yay! Child has the lock and there are enough cookies!\n");
    TracePrintf(1, "Cookies before: %d\n", *(&cookie_jar->cookies));
    cookie_jar->cookies = *(&cookie_jar->cookies) - 5;
    TracePrintf(1, "Cookies after: %d\n", cookie_jar->cookies);
    Release(cookie_jar->lock_id);
    CvarSignal(cookie_jar->cvar_id);
    Exit(0);
  }

  else {
    TracePrintf(1, "I am the parent\n");
    Pause();
    for (int i = 1; i <= 5; i++) {
      while (Acquire(cookie_jar->lock_id) == 0) {
        TracePrintf(1, "Parent has the lock\n");
        if (cookie_jar->cookies >= 5) {
          TracePrintf(1, "Parent has already made enough cookies!\n");
          CvarWait(cookie_jar->cvar_id, cookie_jar->lock_id);
        }
        else {
          break;
        }
      }
      TracePrintf(1, "Parent has the lock. Adding 1 cookie!\n");
      TracePrintf(1, "Cookies before: %d\n", cookie_jar->cookies);
      cookie_jar->cookies = cookie_jar->cookies + 1;
      TracePrintf(1, "Cookies after: %d\n", cookie_jar->cookies);
      Release(cookie_jar->lock_id);
      CvarSignal(cookie_jar->cvar_id);
    }
    Wait(NULL);
    Exit(0);
  }
}