#include "memory_cache.h"
#include <hardware.h>
#include <ylib.h>


memory_cache_t*
memory_cache_new(int num_pages, void* stack_addr)
{
  memory_cache_t* new_memory_cache = malloc(sizeof(memory_cache_t));
  new_memory_cache->num_pages = num_pages;
  new_memory_cache->original_addr = stack_addr;
  new_memory_cache->cache_addr = malloc(PAGESIZE * num_pages);
  return new_memory_cache;
}

void
memory_cache_load(memory_cache_t* memory_cache)
{
  if (memory_cache->cache_addr == NULL) {
    memory_cache->cache_addr = malloc(PAGESIZE * memory_cache->num_pages);
  }
  memcpy(memory_cache->cache_addr, memory_cache->original_addr, memory_cache->num_pages * PAGESIZE);
}

void
memory_cache_restore(memory_cache_t* memory_cache)
{
  memcpy(memory_cache->original_addr, memory_cache->cache_addr, PAGESIZE*memory_cache->num_pages);
}

void
memory_cache_free(memory_cache_t* memory_cache)
{
  free(memory_cache->cache_addr);
  free(memory_cache);
}
