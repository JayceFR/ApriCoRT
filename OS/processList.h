#ifndef __PROCESS_H__ 
#define __PROCESS_H__ 

struct process{
  uint32_t pid; // id 
  uint32_t *page_directory; 
  uint32_t esp, ebp, eip;
  struct process *next; // round robin support
}; 
typedef struct process *process;

struct list;
typedef struct list *list; 
struct list{
  process p;
  list next; 
};

extern process create_process();
extern list create_list();
extern void add_process_list(list head, process p);
extern void print_process_list(list head);

extern uint32_t remove_by_pid(list head, uint32_t pid);

#endif // __PROCESS_H__
