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

void scheduler_init(TCB **_heap);

TCB *alloc_task(unsigned int priority, char name);

void remove_from_ready_queue(TCB *t);

void free_task(TCB *task_ptr);

int Create(int priority, void (*function)());

int MyTid();

int MyParentTid();

void Yield();

void Exit();

#endif /* INC_TASK_H */
