#include <yuser.h>

int main(int argc, char* argv[]) {
  int* ptr = NULL;
  TracePrintf(1, "Attempting to dereference null pointer\n");
  int i = *ptr;
  return 0;
}