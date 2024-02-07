#ifndef _kernel_stack_store_h
#define _kernel_stack_store_h

#include <hardware.h>

typedef struct memory_cache memory_cache_t;

typedef struct memory_cache {
  void* cache_addr;
  int num_pages;
  void* original_addr;
} memory_cache_t;

memory_cache_t* memory_cache_new(int num_pages, void* stack_addr);

// pulls data into the allocated struct
void memory_cache_load(memory_cache_t* memory_cache);

// puts data back where it was copied from
void memory_cache_restore(memory_cache_t* memory_cache);

void memory_cache_free(memory_cache_t* memory_cache);

#endif /*!_kernel_stack_store_h*/
