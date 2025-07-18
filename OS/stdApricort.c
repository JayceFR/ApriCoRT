#include <stdint.h>
#include "stdApricort.h"

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

void *memset(void *s, int c, size_t n) {
  unsigned char *p = (unsigned char *)s;
  while (n--) {
    *p++ = (unsigned char)c;
  }
  return s;
}

/* Clear the screen and initialize VIDEO, XPOS and YPOS. */
void cls (void){
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
void itoa (char *buf, int base, int d){
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
void putchar (int c){
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