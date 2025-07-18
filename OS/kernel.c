#include <multiboot.h>
#include <stdint.h>

#include "stdApricort.h"
#include "pageFrames.h"
#include "kernelHeap.h"

typedef unsigned long size_t;

/* Macros. */

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// Comes from the linker
extern uint32_t kernel_start; 
extern uint32_t kernel_end; 

// BITMAP 
uint8_t page_bitmap[MAX_FRAMES / 8];
uint32_t __attribute__((aligned(4096))) page_directory[1024];
uint32_t __attribute__((aligned(4096))) first_page_table[1024];


/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void cmain (unsigned long magic, unsigned long addr)
{
  multiboot_info_t *mbi;
  
  /* Clear the screen. */
  cls ();

  /* Am I booted by a Multiboot-compliant boot loader? */
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
      printf ("Invalid magic number: 0x%x\n", (unsigned) magic);
      return;
    }

  /* Set MBI to the address of the Multiboot information structure. */
  mbi = (multiboot_info_t *) addr;

  /* Print out the flags. */
  printf ("flags = 0x%x\n", (unsigned) mbi->flags);

  if (CHECK_FLAG (mbi->flags, 6)) {
    multiboot_memory_map_t *mmap;
    
    printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
            (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    
    uint64_t max_addr = 0; 
    
    // Macros to simplify the for loop 
    #define INITMMAP  mmap = (multiboot_memory_map_t *) mbi->mmap_addr
    #define CHECKMAP  (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length
    #define UPDATEMAP mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size))

    for (INITMMAP; CHECKMAP; UPDATEMAP){
      // now we have mmap
      uint64_t region_end = mmap->addr + mmap->len;
      if (region_end > max_addr){
        max_addr = region_end;
      }
    }

    // define the page size 
    uint64_t total_frames = max_addr / PAGE_SIZE;
    size_t bitmap_bytes = (total_frames + 7) / 8; 

    // initialise the page bitmap statically 
    memset(page_bitmap, 0xFF, bitmap_bytes);


    for (INITMMAP; CHECKMAP; UPDATEMAP){
      if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE){

        // Now need to mark the frames as free 
        uint64_t start = mmap->addr;
        uint64_t end   = start + mmap->len; 

        for (uint64_t addr = start; addr < end; addr += PAGE_SIZE){
          uint64_t frame_number = addr / PAGE_SIZE; 
          page_bitmap[frame_number / 8] &= ~(1 << (frame_number % 8));
        }

        printf("Available : base=0x%x%08x length=0x%x%08x\n", 
          (uint32_t)(mmap->addr >> 32), (uint32_t)(mmap->addr & 0xFFFFFFFF), 
          (uint32_t)(mmap->len >> 32), (uint32_t)(mmap->len & 0xFFFFFFFF) );
      }
      else{
        printf("Reserved : base=0x%x%08x length=0x%x%08x\n", 
          (uint32_t)(mmap->addr >> 32), (uint32_t)(mmap->addr & 0xFFFFFFFF), 
          (uint32_t)(mmap->len >> 32), (uint32_t)(mmap->len & 0xFFFFFFFF) );
      }
    }

    // makr frame 0 as reserved 
    page_bitmap[0] |= 1;

    uint32_t k_start = (uint32_t) &kernel_start;
    uint32_t k_end   = (uint32_t) &kernel_end;

    printf("kernel loaded from 0x%x to 0x%x\n", k_start, k_end);

    // now mark the frames occupied by the kernel 
    for (uint32_t addr = k_start; addr < k_end; addr += PAGE_SIZE){
      uint32_t frame = addr / PAGE_SIZE; 
      page_bitmap[frame / 8] |= (1 << (frame % 8));
    }

  }

  // Paging stufff Ooooff

  for (int i = 0; i < 1024; i++){
    first_page_table[i] = (i * 0x1000) | 3;
  }

  page_directory[0] = ((uint32_t) first_page_table) | 3;

  // Enable paging 

  // Load page directory
  asm volatile("mov %0, %%cr3" :: "r"(page_directory));

  // Enable paging (set the PG and PE bits in CR0)
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000; // Set PG bit
  asm volatile("mov %0, %%cr0" :: "r"(cr0));

  init_kernel_heap(page_directory);

  // Test cases 

  // Fail text case 
  // uint32_t *ptr = (uint32_t*)0x400000; // 4MB mark (just above what you mapped)
  // *ptr = 42;

  printf("Test for frames");
  void *a = alloc_frame();
  void *b = alloc_frame();
  void *c = alloc_frame();

  printf(" a = 0x%x\n", (uint32_t)(uintptr_t)a);
  printf(" b = 0x%x\n", (uint32_t)(uintptr_t)b);
  printf(" c = 0x%x\n", (uint32_t)(uintptr_t)c);

  free_frame(b);
  printf("Freed frame b\n");

  void *d = alloc_frame();
  printf("Allocated d = 0x%x\n", (uint32_t)(uintptr_t)d);

  void *arr = alloc_n_frames(5);
  printf(" arr = 0x%x\n", (uint32_t)(uintptr_t)arr);

  void *e = alloc_frame();
  printf("Allocated e = 0x%x\n", (uint32_t)(uintptr_t)e);

  void *am = kmalloc(20);
  void *bm = kmalloc(64);

  printf("am = 0x%x\n bm = 0x%x\n", (uint32_t)(uintptr_t)am, (uint32_t)(uintptr_t)bm);

}    