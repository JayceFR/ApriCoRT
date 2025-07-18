#ifndef __PROCESS_H__ 
#define __PROCESS_H__ 

typedef enum {
  READY,
  RUNNING,
  WAITING, 
  TERMINATED,
} pstate;

struct process{
  uint32_t pid; // id 
  uint32_t state; // ready, running, waiting, etc. 
  uint32_t *page_directory; 
  uint32_t esp, ebp, eip;

  uint8_t isUser; 
  void *stackTop;  
}; 
typedef struct process *process;

struct list;
typedef struct list *list; 
struct list{
  process p;
  list next; 
};

#define PROCESS_STACK_SIZE 4096 // 4KB

extern process create_process();
extern list create_list();
extern void add_process_list(list head, process p);
extern void print_process_list(list head);

extern uint32_t remove_by_pid(list head, uint32_t pid);

#endif // __PROCESS_H__
