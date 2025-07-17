#include <multiboot.h>
#include <stdint.h>

#include "stdApricort.h"
#include "pageFrames.h"
#include "kernelHeap.h"

typedef unsigned long size_t;

/* Macros. */

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/* Some screen stuff. */
/* The number of columns. */
#define COLUMNS                 80
/* The number of lines. */
#define LINES                   24
/* The attribute of an character. */
#define ATTRIBUTE               7
/* The video memory address. */
#define VIDEO                   0xB8000

/* Variables. */
/* Save the X position. */
static int xpos;
/* Save the Y position. */
static int ypos;
/* Point to the video memory. */
static volatile unsigned char *video;

// Comes from the linker
extern uint32_t kernel_start; 
extern uint32_t kernel_end; 

// BITMAP 
uint8_t page_bitmap[MAX_FRAMES / 8];
uint32_t __attribute__((aligned(4096))) page_directory[1024];
uint32_t __attribute__((aligned(4096))) first_page_table[1024];

/* Forward declarations. */
void cmain (unsigned long magic, unsigned long addr);
static void cls (void);
static void itoa (char *buf, int base, int d);
static void putchar (int c);
void printf (const char *format, ...);


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

  /* Are mem_* valid? */
  if (CHECK_FLAG (mbi->flags, 0))
    printf ("mem_lower = %uKB, mem_upper = %uKB\n",
            (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

  /* Is boot_device valid? */
  if (CHECK_FLAG (mbi->flags, 1))
    printf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
  
  /* Is the command line passed? */
  if (CHECK_FLAG (mbi->flags, 2))
    printf ("cmdline = %s\n", (char *) mbi->cmdline);

  /* Are mods_* valid? */
  if (CHECK_FLAG (mbi->flags, 3))
    {
      multiboot_module_t *mod;
      int i;
      
      printf ("mods_count = %d, mods_addr = 0x%x\n",
              (int) mbi->mods_count, (int) mbi->mods_addr);
      for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
           i < mbi->mods_count;
           i++, mod++)
        printf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
                (unsigned) mod->mod_start,
                (unsigned) mod->mod_end,
                (char *) mod->cmdline);
    }

  /* Bits 4 and 5 are mutually exclusive! */
  if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
    {
      printf ("Both bits 4 and 5 are set.\n");
      return;
    }

  /* Is the symbol table of a.out valid? */
  if (CHECK_FLAG (mbi->flags, 4))
    {
      multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);
      
      printf ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
              "strsize = 0x%x, addr = 0x%x\n",
              (unsigned) multiboot_aout_sym->tabsize,
              (unsigned) multiboot_aout_sym->strsize,
              (unsigned) multiboot_aout_sym->addr);
    }

  /* Is the section header table of ELF valid? */
  if (CHECK_FLAG (mbi->flags, 5))
    {
      multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

      printf ("multiboot_elf_sec: num = %u, size = 0x%x,"
              " addr = 0x%x, shndx = 0x%x\n",
              (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
              (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
    }

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

  printf("am = %p\n bm = %p\n", am, bm);

}    

/* Clear the screen and initialize VIDEO, XPOS and YPOS. */
static void cls (void){
  int i;

  video = (unsigned char *) VIDEO;
  
  for (i = 0; i < COLUMNS * LINES * 2; i++)
    *(video + i) = 0;

  xpos = 0;
  ypos = 0;
}

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to ’d’, interpret that D is decimal, and if BASE is
   equal to ’x’, interpret that D is hexadecimal. */
static void itoa (char *buf, int base, int d){
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
  
  /* If %d is specified and D is minus, put ‘-’ in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
      
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);

  /* Terminate BUF. */
  *p = 0;
  
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2){
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }
}

/* Put the character C on the screen. */
static void
putchar (int c){
  if (c == '\n' || c == '\r'){
    newline:
    xpos = 0;
    ypos++;
    if (ypos >= LINES)
      ypos = 0;
    return;
  }

  *(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
  *(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

  xpos++;
  if (xpos >= COLUMNS)
    goto newline;
}

/* Format a string and print it on the screen, just like the libc
   function printf. */
void printf (const char *format, ...){
  char **arg = (char **) &format;
  int c;
  char buf[20];

  arg++;
  
  while ((c = *format++) != 0){
    if (c != '%')
      putchar (c);
    else {
      char *p, *p2;
      int pad0 = 0, pad = 0;
      
      c = *format++;
      if (c == '0'){
        pad0 = 1;
        c = *format++;
      }

      if (c >= '0' && c <= '9'){
        pad = c - '0';
        c = *format++;
      }

      switch (c){
        case 'd':
        case 'u':
        case 'x':
          itoa (buf, c, *((int *) arg++));
          p = buf;
          goto string;
          break;

        case 's':
          p = *arg++;
          if (! p)
            p = "(null)";

        string:
          for (p2 = p; *p2; p2++);
          for (; p2 < p + pad; p2++)
            putchar (pad0 ? '0' : ' ');
          while (*p)
            putchar (*p++);
          break;

        default:
          putchar (*((int *) arg++));
          break;
      }
    }
  }
}