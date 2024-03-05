#include "util.h"
#include "hardware.h"

// HELPER FUNCTION:
// returns an allocated array of size 2
// first bit contains memory region of input addr
// second bit contains the page number in the
// corresponding page table
int
get_raw_page_no(void* addr)
{
  int page_no = (unsigned int)addr/PAGESIZE;
  return page_no;
}
