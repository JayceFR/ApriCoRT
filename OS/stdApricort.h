#ifndef STD_APRICORT_H
#define STD_APRICORT_H

#define MAX_FRAMES (1024 * 1024)
#define PAGE_SIZE 4096 // 4KB
#define NULL 0

typedef unsigned long size_t; 


extern void *memset(void *s, int c, size_t n) ;

extern void printf (const char *format, ...);

extern void cls (void);
extern void putchar (int c);

extern void itoa (char *buf, int base, int d);

#endif // __STDAPRI_H__
