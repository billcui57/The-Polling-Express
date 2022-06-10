#include <context.h>
#include <hal.h>
#include <kprintf.h>
#include <my_assert.h>
#include <my_event.h>
#include <syscall.h>
#include <task.h>
#include <timer.h>
#include <user.h>

int msg_copy(const char *src, int srclen, char *dest, int destlen) {
  int n = srclen < destlen ? srclen : destlen;
  for (int i = 0; i < n; i++)
    dest[i] = src[i];
  return n;
}

void handle_send(TCB *src, TCB *dst) {
  src->state = REPLY;
  dst->state = READY;
  send_args *s_args = (send_args *)get_data(&src->context);
  receive_args *r_args = (receive_args *)get_data(&dst->context);
  *r_args->tid = src->tid;
  int ret = msg_copy(s_args->msg, s_args->msglen, r_args->msg, r_args->msglen);
  set_return(&dst->context, ret);
  add_to_ready_queue(dst);
}

void kmain() {
  uart_init(COM2);

  enable_cache();

  TCB *event_mapping[NUMBER_OF_EVENTS];
  for (unsigned int i = 0; i < NUMBER_OF_EVENTS; i++) {
    event_mapping[i] = NULL;
  }

  enable_irq();

  timer_init(TIMER1); // for ticks, interrupt enabled
  start_timer(TIMER1);

  timer_init(TIMER3); // general purpose, interrupt disabled
  start_timer(TIMER3);

  TCB backing[MAX_NUM_TASKS];
  TCB *heap[MAX_NUM_TASKS];
  scheduler_init(MAX_NUM_TASKS, backing, heap);

  __asm__ volatile("MRS R0, CPSR\n\t"
                   "BIC R0, R0, #0x1F\n\t"
                   "ORR R0, R0, #0x12\n\r"
                   "MSR CPSR_c, R0\n\r"
                   "MOV SP, %[irq_stack]\n\r"
                   "ORR R0, R0, #1\n\r"
                   "MSR CPSR_c, R0\n\r" ::[irq_stack] "r"(&irq_stack[15])
                   : "r0");

  *((void (**)())0x28) = &return_swi;
  *((void (**)())0x38) = &return_irq;

  int interrupt_tasks = 0;

  scheduler_add(-99, idle, -1);
  scheduler_add(0, task_k3init, -1);

  while (scheduler_length() > 1 || interrupt_tasks != 0) {

    TCB *cur = pop_ready_queue();

    int why = run_user(&cur->context);

    int data = get_data(&cur->context);
    if (why == SYSCALL_CREATE) {
      create_args *args = (create_args *)data;
      int ret = scheduler_add(args->priority, args->function, cur->tid);
      set_return(&cur->context, ret);
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_MYTID) {
      set_return(&cur->context, cur->tid);
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_MYPARENTTID) {
      set_return(&cur->context, cur->parentTid);
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_YIELD) {
      add_to_ready_queue(cur);
    } else if (why == SYSCALL_EXIT) {
      cur->state = ZOMBIE;
    } else if (why == SYSCALL_SEND) {
      send_args *args = (send_args *)data;

      if ((args->tid < 0) || (args->tid > MAX_NUM_TASKS)) {
        set_return(&cur->context, EINVALIDTID);
        add_to_ready_queue(cur);
      } else {
        TCB *target = &backing[args->tid];
        if (target->state == ZOMBIE) {
          set_return(&cur->context, EINVALIDTID);
          add_to_ready_queue(cur);
        } else {
          TCB *target = &backing[args->tid];
          if (target->state == ZOMBIE) {
            set_return(&cur->context, EINVALIDTID);
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
        }
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
        set_return(&cur->context, EINVALIDTID);
        add_to_ready_queue(cur);
      } else if (target->state != REPLY) {
        set_return(&cur->context, ENOTREPLYWAIT);
        add_to_ready_queue(cur);
      } else {
        send_args *s_args = (send_args *)get_data(&target->context);
        target->state = READY;
        int ret =
            msg_copy(args->reply, args->rplen, s_args->reply, s_args->rplen);
        set_return(&target->context, ret);
        add_to_ready_queue(target);
        set_return(&cur->context, ret);
        add_to_ready_queue(cur);
      }
    } else if (why == SYSCALL_AWAITEVENT) {

      int event = data;

      KASSERT(event_mapping[event] == NULL,
              "No other task should be waiting for this event");

      if (event == BREAK_IDLE) {
        unsigned int start_time;
        read_timer(TIMER3, &start_time);
        __asm__ volatile("MCR p15,0,%[zero],c7,c0,4" ::[zero] "r"(0));
        unsigned int end_time;
        read_timer(TIMER3, &end_time);
        set_return(&cur->context, start_time - end_time);
        add_to_ready_queue(cur);
      } else {
        event_mapping[event] = cur;
        interrupt_tasks++;
      }

    } else if (why == SYSCALL_IRQ) {

      // now we determine who caused this interrupt

      int vic1_irq_status = *(volatile int *)(VIC1_BASE + IRQ_STAT_OFFSET);
      int vic2_irq_status = *(volatile int *)(VIC2_BASE + IRQ_STAT_OFFSET);

      // printf(COM2, "%d\r\n", vic1_irq_status);
      // printf(COM2, "%d\r\n", vic2_irq_status);

      if (vic1_irq_status & VIC_TIMER1_MASK) {
        *(int *)(TIMER1_BASE + CLR_OFFSET) = 1; // clear timer interrupt
        if (event_mapping[TIMER_TICK] != NULL) {
          TCB *waiting_task = event_mapping[TIMER_TICK];
          event_mapping[TIMER_TICK] = NULL;
          interrupt_tasks--;
          set_return(&(waiting_task->context), 0);
          add_to_ready_queue(waiting_task);
        }
      }

      add_to_ready_queue(cur);

    } else {
      KASSERT(0, "Unknown Syscall\r\n");
    }
  }
}

// void idle() {
//   // __asm__ volatile("ldr r0, =80930000\n\t" // Syscon base address
//   //                  "mov r1, #0xaa\n\t"
//   //                  "str r1, [r0, #0xc0]\n\r"
//   //                  "ldr r1, [r0, #0x80]\n\r"
//   //                  "orr r1, r1, #0x1\n\r"
//   //                  "str r1, [r0, #0x80]\n\r");

//   int *lock = (int *)(SYSCON_BASE + SW_LOCK_OFFSET);

//   *lock = 0xAA; // opens lock

//   int *device_cfg = (int *)(SYSCON_BASE + DEVICE_CFG_OFFSET);

//   *device_cfg = (*device_cfg) | SHENA_MASK;

//   int *halt = (int *)(SYSCON_BASE + HALT_OFFSET);

//   while (true) {
//     *halt; // request to halt
//   }
// }
