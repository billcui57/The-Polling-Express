#ifndef INC_TASK_H
#define INC_TASK_H

#include <stddef.h>

#define EINVALIDPRIORITY -1   // invalid priority
#define ENOTASKDESCRIPTORS -2 // kernel is out of task descriptors

typedef struct mp_node
{
  mp_node *next;
  TCB t;
} mp_node;

typedef struct memory_pool
{
  mp_node *nodes;
  mp_node *nodes_end;
  mp_node *head;
  size_t capacity;
  size_t count; // num of tasks
} memory_pool;

memory_pool mp;

typedef struct TCB
{
  char name;
  int val;
} TCB;

int alloc_task(TCB task, TCB *task_ptr);

int free_task(TCB *task_ptr);

int Create(int priority, void (*function)());

int MyTid();

int MyParentTid();

void Yield();

void Exit();

#endif /* INC_TASK_H */