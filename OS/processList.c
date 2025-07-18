// A linked list implementation to manage processes 
// Planning to implement a circular linked list 

#include <stdint.h>
#include "stdApricort.h"
#include "kernelHeap.h"
#include "processList.h"

process createProcess(){
  process p = kmalloc(sizeof(struct process));
  if (p == NULL){
    printf("insufficent memory to create a process");
    return NULL;
  }
  return p;
}

list createList(){
  list l = kmalloc(sizeof(struct list));
  if (l == NULL){
    printf("insufficient memory to create the process list");
    return NULL;
  }

  l->p    = NULL;
  l->next = NULL;

  return l; 
}


void add(list head, process p){
  if (head->p == NULL){
    head->p = p; 
    return; 
  }
  list curr = head;
  while(curr->next != NULL){
    curr = curr->next;
  }
  curr->next = createList();
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
