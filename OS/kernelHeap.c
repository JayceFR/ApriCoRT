#include <stdint.h>

#include "stdApricort.h"
#include "pageFrames.h"
#include "kernelHeap.h"

#define KERNEL_HEAP_START 0xC0000000  // 3GB
#define KERNEL_HEAP_SIZE  0x10000     // 1MB


// map the virtual pages, so kernel can use them 
void init_kernel_heap(uint32_t *page_dir){
  for (uint32_t addr = KERNEL_HEAP_START; addr < KERNEL_HEAP_START + KERNEL_HEAP_SIZE; addr += PAGE_SIZE){
    void *frame = alloc_frame();
    map_page(addr, (uint32_t)(uintptr_t) frame, 0x3, page_dir);
  }
}

static uint32_t heap_curr = KERNEL_HEAP_START;
static uint32_t heap_end  = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;

void *kmalloc(size_t size){
  size = (size + 7) & ~7;
  if (heap_curr + size > heap_end){
    // grow dynamically if needed
    return NULL;
  }

  void *addr = (void *)heap_curr;
  heap_curr += size;
  return addr;
}