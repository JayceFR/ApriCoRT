#ifndef __PROCESS_H__ 
#define __PROCESS_H__ 

struct process{
  uint32_t pid; // id 
  uint32_t *page_directory; 
  uint32_t esp, ebp, eip;
  struct process *next; // round robin support
}; 
typedef struct process *process;

typedef struct list{
  process p;  
  struct list *next; 
} *list;

extern process createProcess();
extern list createList();
extern void add(list head, process p);
extern void print_process_list(list head);

#endif // __PROCESS_H__
