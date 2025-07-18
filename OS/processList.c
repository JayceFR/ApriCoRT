// A linked list implementation to manage processes 
// Planning to implement a circular linked list 

#include <stdint.h>
#include "stdApricort.h"
#include "kernelHeap.h"
#include "processList.h"

static uint32_t processId = 0;

// Creates and returns a new process 
// The pid is automatically set for each process. 
process create_process(void (*entryPoint)(), uint8_t isUser){
  process p = kmalloc(sizeof(struct process));
  if (p == NULL){
    printf("insufficent memory to create a process");
    return NULL;
  }
  p->pid = processId++;
  p->state = READY;
  p->isUser = isUser;
  // p->page_directory = 

  // Allocate stack 
  void *stack = kmalloc(PROCESS_STACK_SIZE);
  if (stack == NULL){
    printf("Failed to allocated stack\n");
    return NULL;
  }

  p->stackTop = stack;

  p->esp = (uint32_t) stack + PROCESS_STACK_SIZE;
  p->ebp = p->esp; // for old computers 

  // Entry point 
  p->eip = (uint32_t) entryPoint;

  return p;
}

list create_list(){
  list l = kmalloc(sizeof(struct list));
  if (l == NULL){
    printf("insufficient memory to create the process list");
    return NULL;
  }

  l->p    = NULL;
  l->next = NULL;

  return l; 
}


void add_process_list(list head, process p){
  if (head->p == NULL){
    head->p = p; 
    return; 
  }
  list curr = head;
  while(curr->next != NULL){
    curr = curr->next;
  }
  curr->next = create_list();
  curr->next->p = p; 
}

void print_process_list(list head){
  if (head->p == NULL){
    printf("Empty list");
    return; 
  }
  list curr = head;
  while(curr != NULL){
    printf("Process id = %d\n", curr->p->pid);
    curr = curr->next;
  }
}

static void removeHead(list head){
  if (head == NULL){
    return;
  }
  *head = *(head->next);
  // need to free temp
}

// Removes the process from the list
// Returns the removed processes' pid 
// Returns NULL if not found
uint32_t remove_by_pid(list head, uint32_t pid){
  if (head->p == NULL){
    return NULL;
  }

  if (head->p->pid == pid){
    removeHead(head);
    return pid;
  }

  // find the position of the node 
  list curr = head; 
  for (; curr->next != NULL && curr->next->p->pid != pid; curr = curr->next){
    /* EMPTY BODY*/
  }
  
  if (curr->next == NULL){
    return NULL;
  }

  // need to remove current's next 
  curr->next = curr->next->next;
  return pid;
}
