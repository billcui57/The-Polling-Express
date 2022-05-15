#ifndef INC_TASK_H
#define INC_TASK_H

#include <context.h>
#include <math.h>
#include <stddef.h>

#define EINVALIDPRIORITY -1   // invalid priority
#define ENOTASKDESCRIPTORS -2 // kernel is out of task descriptors

typedef enum tcb_state {
  ACTIVE,
  READY,
  ZOMBIE,
  SEND,
  RECEIVE,
  REPLY,
  EVENT
} tcb_state;

typedef struct TCB {
  // TCB is free
  struct TCB *next;

  // TCB contains valid task
  int tid;
  int priority;
  tcb_state state;
  int parentTid;
  user_task context;
} TCB;

void scheduler_init(size_t cap, TCB *blocks, TCB **ready_queue);

int scheduler_add(int priority, void (*func)(), int parentTid);

void add_to_ready_queue(TCB *t);

TCB *pop_ready_queue();

void free_task(TCB *task_ptr);

bool scheduler_empty();
#endif /* INC_TASK_H */
