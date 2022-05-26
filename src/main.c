#include <context.h>
#include <hal.h>
#include <kprintf.h>
#include <my_assert.h>
#include <syscall.h>
#include <task.h>
#include <timer.h>
#include <user.h>

uart pc;

int msg_copy(const char *src, int srclen, char *dest, int destlen) {
  int n = srclen < destlen ? srclen : destlen;
  for (int i = 0; i < n; i++)
    dest[i] = src[i];
  return n;
}

void handle_send(TCB *src, TCB *dst) {
  src->state = REPLY;
  dst->state = READY;
  send_args *s_args = (send_args *)get_data(&src->context.reg);
  receive_args *r_args = (receive_args *)get_data(&dst->context.reg);
  *r_args->tid = src->tid;
  int ret = msg_copy(s_args->msg, s_args->msglen, r_args->msg, r_args->msglen);
  set_return(&dst->context.reg, ret);
  add_to_ready_queue(dst);
}

void kmain() {
  timer t;
  timer_init(&t, TIMER3);
  start_timer(&t);

  uart_init(&pc, COM2);
  TCB backing[MAX_NUM_TASKS];
  TCB *heap[MAX_NUM_TASKS];
  scheduler_init(MAX_NUM_TASKS, backing, heap, &t);

  assert_init(&pc);

  *((void (**)())0x28) = &return_swi;

  scheduler_add(0, task_k1init, -1);

  while (!scheduler_empty()) {
    TCB *cur = pop_ready_queue();
    int why = run_user(&cur->context.reg);
    int data = get_data(&cur->context.reg);
    if (why == SYSCALL_CREATE) {
      create_args *args = (create_args *)data;
      int ret = scheduler_add(args->priority, args->function, cur->tid);
      set_return(&cur->context.reg, ret);
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_MYTID) {
      set_return(&cur->context.reg, cur->tid);
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_MYPARENTTID) {
      set_return(&cur->context.reg, cur->parentTid);
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_YIELD) {
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_EXIT) {
      cur->state = ZOMBIE;
    } else if (why == SYSCALL_SEND) {
      send_args *args = (send_args *)data;
      TCB *target = &backing[args->tid];
      if (target->state == ZOMBIE) {
        set_return(&cur->context.reg, EINVALIDTID);
        add_to_ready_queue(cur);
      } else if (target->state == RECEIVE) {
        handle_send(cur, target);
      } else {
        cur->state = SEND;
        cur->next = NULL;
        if (target->want_send_end) {
          target->want_send_end->next = cur;
        } else {
          target->want_send = cur;
        }
        target->want_send_end = cur;
      }
    } else if (why == SYSCALL_RECEIVE) {
      if (cur->want_send) {
        TCB *src = cur->want_send;
        cur->want_send = src->next;
        if (!cur->want_send)
          cur->want_send_end = NULL;
        src->next = NULL;

        handle_send(src, cur);
      } else {
        cur->state = RECEIVE;
      }
    } else if (why == SYSCALL_REPLY) {
      reply_args *args = (reply_args *)data;
      TCB *target = &backing[args->tid];
      if (target->state == ZOMBIE) {
        set_return(&cur->context.reg, EINVALIDTID);
        add_to_ready_queue(cur);
      } else if (target->state != REPLY) {
        set_return(&cur->context.reg, ENOTREPLYWAIT);
        add_to_ready_queue(cur);
      } else {
        send_args *s_args = (send_args *)get_data(&target->context.reg);
        target->state = READY;
        int ret =
            msg_copy(args->reply, args->rplen, s_args->reply, s_args->rplen);
        set_return(&target->context.reg, ret);
        add_to_ready_queue(target);
        set_return(&cur->context.reg, ret);
        add_to_ready_queue(cur);
      }
    } else {
      KASSERT(0, "Unknown Syscall\r\n");
    }
  }
}
