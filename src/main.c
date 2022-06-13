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

bool wake_up(int event_id, TCB **event_mapping, int *interrupt_tasks) {
  if (event_mapping[event_id] != NULL) {
    TCB *waiting_task = event_mapping[event_id];
    event_mapping[event_id] = NULL;
    (*interrupt_tasks)--;
    set_return(&(waiting_task->context), 0);
    add_to_ready_queue(waiting_task);
    return true;
  }
  return false;
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
  scheduler_add(100, task_k4_init, -1);

  while (scheduler_length() > 1 || interrupt_tasks != 0) {

    // printf(BW_COM2, "[Vic1 enable] %d\r\n",
    //        *(int *)(VIC1_BASE + INT_ENABLE_OFFSET));

    // printf(BW_COM2, "[UART ctrl] %d\r\n",
    //        *((int *)(get_base_addr(COM2) + UART_CTLR_OFFSET)));

    TCB *cur = pop_ready_queue();

    // printf(BW_COM2, "[Tid]: %d \t [Priority]: %d \r\n", cur->tid,
    //        cur->priority);
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
      // printf(BW_COM2, "[Exit] [Tid]: %d \t [Priority]: %d \r\n", cur->tid,
      //        cur->priority);
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
      } else if (event == UART1_INTR) {
        enable_interrupt(UART1INTR);
        event_mapping[event] = cur;
        interrupt_tasks++;
      } else if (event == UART1_RX_INTR) {
        enable_interrupt(UART1RXINTR);
        event_mapping[event] = cur;
        interrupt_tasks++;
      } else if (event == UART2_TX_HALF_EMPTY) {
        enable_interrupt(UART2TXINTR);
        event_mapping[event] = cur;
        interrupt_tasks++;
      }

    } else if (why == SYSCALL_IRQ) {

      // now we determine who caused this interrupt

      int vic1_irq_status = *(volatile int *)(VIC1_BASE + IRQ_STAT_OFFSET);
      int vic2_irq_status = *(volatile int *)(VIC2_BASE + IRQ_STAT_OFFSET);

      if (vic1_irq_status & VIC_TIMER1_MASK) {
        *(int *)(TIMER1_BASE + CLR_OFFSET) = 1; // clear timer interrupt
        wake_up(TIMER_TICK, event_mapping, &interrupt_tasks);
      }

      if (vic1_irq_status & VIC_UART2TXINTR_MASK) {

        // cannot clear a level interrupt
        if (wake_up(UART2_TX_HALF_EMPTY, event_mapping, &interrupt_tasks)) {
          // bw_uart_put_char(COM2, 'D');
          disable_interrupt(UART2TXINTR);
        } else {
          KASSERT(0, "Nobody home");
        }
        // bw_uart_put_char(COM2, 'I');
      }

      int *uart1_ctrl = (int *)(get_base_addr(COM1) + UART_CTLR_OFFSET);
      volatile int *uart1_intr =
          (int *)(get_base_addr(COM1) + UART_INTR_OFFSET);

      if (vic1_irq_status & VIC_UART1RXINTR_MASK) {
        if (wake_up(UART1_RX_INTR, event_mapping, &interrupt_tasks)) {
          disable_interrupt(UART1RXINTR);
          *uart1_ctrl = *uart1_ctrl & ~RIEN_MASK;
        }
      }

      if ((vic2_irq_status & VIC_INT_UART1_MASK) && (*uart1_intr & ~RIS_MASK)) {
        if (wake_up(UART1_INTR, event_mapping, &interrupt_tasks)) {
          disable_interrupt(UART1INTR);
        }
      }

      add_to_ready_queue(cur);

    } else {
      KASSERT(0, "Unknown Syscall\r\n");
    }
  }
}
