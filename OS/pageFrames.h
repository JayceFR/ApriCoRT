#ifndef __PAGEFRA_H__ 
#define __PAGEFRA_H__ 

#include <stdint.h>

extern void *alloc_frame();
extern void *alloc_n_frames(uint64_t n);
extern void free_frame(void *ptr);

#endif // __PAGEFRA_H__
