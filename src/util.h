#ifndef _util_h
#define _util_h

// HELPER FUNCTION:
// returns an allocated array of size 2
// first bit contains memory region of input addr
// second bit contains the page number in the
// corresponding page table
int get_raw_page_no(void* addr);

#endif /*!_util_h*/