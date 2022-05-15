#ifndef INC_TASK_H
#define INC_TASK_H

#include <math.h>
#include <stddef.h>

#define EINVALIDPRIORITY -1   // invalid priority
#define ENOTASKDESCRIPTORS -2 // kernel is out of task descriptors

typedef int Priority_t;

typedef void (*TaskFunction_t)(void *);

typedef struct TCB {
  // TCB is free
  struct TCB *next;

  // TCB contains valid task
  char name;
  Priority_t priority;

} TCB;

void memory_pool_init(size_t cap, TCB *blocks);

void scheduler_init(TCB **ready_queue);

TCB *alloc_task(unsigned int priority, char name);

void add_to_ready_queue(TCB *t);

TCB *pop_ready_queue();

void free_task(TCB *task_ptr);
#endif /* INC_TASK_H */
