#ifndef INC_TASK_H
#define INC_TASK_H

#include <stddef.h>

#define EINVALIDPRIORITY -1   // invalid priority
#define ENOTASKDESCRIPTORS -2 // kernel is out of task descriptors

typedef int Priority_t;

typedef void (*TaskFunction_t)(void *);

typedef struct TCB {
  struct TCB *next;
  char name;
  int val;
  Priority_t priority;
} TCB;

void memory_pool_init(size_t capacity, TCB *blocks);

TCB *alloc_task();

void free_task(TCB *task_ptr);

int Create(int priority, void (*function)());

int MyTid();

int MyParentTid();

void Yield();

void Exit();

#endif /* INC_TASK_H */
