#ifndef INC_TASK_H
#define INC_TASK_H

#include <context.h>
#include <math.h>
#include <stddef.h>
#include <timer.h>

#define MAX_NUM_TASKS 10

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

  unsigned int added_time;

  struct TCB *want_send;
  struct TCB *want_send_end;
} TCB;

void scheduler_init(size_t cap, TCB *blocks, TCB **ready_queue, timer *t);

int scheduler_add(int priority, void (*func)(), int parentTid);

void add_to_ready_queue(TCB *t);

TCB *pop_ready_queue();

void free_task(TCB *task_ptr);

bool scheduler_empty();
#endif /* INC_TASK_H */
