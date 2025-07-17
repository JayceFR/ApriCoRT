// Contains functions to allocate and free page frames from memory 

#include <stdint.h>
#include "stdApricort.h"

extern uint8_t *page_bitmap;

// Returns the next free memory location. 
// Returns 0 (NULL) if it is out of memory 
void *alloc_frame(){
  for (uint64_t i = 0; i < MAX_FRAMES; i++){
    uint64_t byte_index = i / 8; 
    uint8_t bit_index = i % 8; 

    if (!(page_bitmap[byte_index] & (1 << bit_index))){
      // Mark the frame used
      page_bitmap[byte_index] |= (1 << bit_index);
      // return the address 
      uint64_t addr = i * PAGE_SIZE;
      return (void *)(uintptr_t) addr;
    }

  }
  // Out of memory lol 
  return 0;
}

// Allocates n contiguous frames
void *alloc_n_frames(uint64_t n){
  // need to find n contiguous free frames in memory 
  for (uint64_t i = 0; i < MAX_FRAMES; i++){
    uint8_t flag = 0x0; 
    for (uint64_t start = i; start < i + n; start ++){
      uint64_t byte_index = start / 8; 
      uint8_t bit_index = start % 8;

      if (page_bitmap[byte_index] & (1 << bit_index)){
        flag = 0x1; 
      }
    }

    if (flag == 0x0){
      // set all the bits to 1
      for (uint64_t start = i; start < i + n; start ++){
        uint64_t byte_index = start / 8; 
        uint8_t bit_index = start % 8;

        page_bitmap[byte_index] |= (1 << bit_index);
      }
      uint64_t addr = i * PAGE_SIZE;
      return (void *)(uintptr_t) addr;
    }
  }
  // no contiguous memory is found 
  return 0; 
}

// Frees the frame
void free_frame(void *ptr){
  uint64_t addr   = (uintptr_t) ptr;
  uint64_t frame  = addr / PAGE_SIZE;
  uint64_t byte_i = frame / 8; 
  uint8_t  bit_i  = frame % 8; 

  page_bitmap[byte_i] &= ~(1 << bit_i);
}