#ifndef __KERNELH_H__ 
#define __KERNELH_H__

#include <stdint.h>
#include "stdApricort.h"

extern void init_kernel_heap(uint32_t *page_dir);
extern void *kmalloc(size_t size);

#endif // __KERNELH_H__
