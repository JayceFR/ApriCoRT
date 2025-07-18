#ifndef __PAGEFRA_H__ 
#define __PAGEFRA_H__ 

#include <stdint.h>

extern void *alloc_frame();
extern void *alloc_n_frames(uint64_t n);
extern void free_frame(void *ptr);

extern void map_page(uint32_t virt_addr, uint32_t phy_addr, uint32_t flags, uint32_t *page_dir);

extern void init_paging(uint32_t *page_directory);

#endif // __PAGEFRA_H__
