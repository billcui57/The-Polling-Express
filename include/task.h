#ifndef INC_TASK_H
#define INC_TASK_H

#include <stddef.h>

#define EINVALIDPRIORITY -1   // invalid priority
#define ENOTASKDESCRIPTORS -2 // kernel is out of task descriptors

typedef struct TCB {
  struct TCB *next;
  char name;
  int val;
} TCB;

void memory_pool_init(size_t capacity, TCB *blocks);

TCB *alloc_task();

void free_task(TCB *task_ptr);
#endif /* INC_TASK_H */
