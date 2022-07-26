#include "debugprinter.h"
#include <context.h>
#include <hal.h>
#include <kprintf.h>
#include <my_assert.h>
#include <my_event.h>
#include <syscall.h>
#include <task.h>
#include <timer.h>
#include <user.h>

extern char __bss_start, __bss_end; // defined in linker script

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
  memset(&__bss_start, 0, &__bss_end - &__bss_start);
  while (uart_can_read(COM1))
    uart_get_char(COM1);
  (*(volatile int *)(get_base_addr(COM1) + UART_RSR_OFFSET)) = 0;

  int org_exc_stack, org_swi_vec, org_irq_vec;
  __asm__ volatile("MRS R0, CPSR\n\t"
                   "BIC R0, R0, #0x1F\n\t"
                   "ORR R0, R0, #0x12\n\r"
                   "MSR CPSR_c, R0\n\r"
                   "MOV %[org_exc_stack], SP\n\r"
                   "MOV SP, %[irq_stack]\n\r"
                   "ORR R0, R0, #1\n\r"
                   "MSR CPSR_c, R0\n\r"
                   : [org_exc_stack] "=&r"(org_exc_stack)
                   : [irq_stack] "r"(&irq_stack[15])
                   : "r0");

  org_swi_vec = *(volatile int *)0x28;
  org_irq_vec = *(volatile int *)0x38;
  *((void (**)())0x28) = &return_swi;
  *((void (**)())0x38) = &return_irq;

  which_track = '?';
  if (org_exc_stack) { // not emulator
    unsigned int mac = *(unsigned int *)0x80010054;
    char c = ((mac == 0x0e6d) ? 'a' : ((mac == 0xc5da) ? 'b' : '?'));
    which_track = c;
  }

  uart_init(COM2);

  enable_cache();

  TCB *event_mapping[NUMBER_OF_EVENTS];
  for (unsigned int i = 0; i < NUMBER_OF_EVENTS; i++) {
    event_mapping[i] = NULL;
  }

  int uart1_tx_flag = 0;
  enable_irq();

  timer_init(TIMER1); // for ticks, interrupt enabled
  start_timer(TIMER1);

  timer_init(TIMER3); // general purpose, interrupt disabled
  start_timer(TIMER3);

  TCB backing[MAX_NUM_TASKS];
  TCB *heap[MAX_NUM_TASKS];
  scheduler_init(MAX_NUM_TASKS, backing, heap);

  int interrupt_tasks = 0;

  scheduler_add(-99, idle, -1, "idle");
  scheduler_add(100, task_k4_init, -1, "k1init");

  while (scheduler_length() > 1 || interrupt_tasks != 0) {
    KASSERT(
        !((*(volatile int *)(get_base_addr(COM1) + UART_RSR_OFFSET)) & OE_MASK),
        "Dropped sensor byte");

    // printf(BW_COM2, "[Vic1 enable] %d\r\n",
    //        *(int *)(VIC1_BASE + INT_ENABLE_OFFSET));

    // printf(BW_COM2, "[UART ctrl] %d\r\n",
    //        *((int *)(get_base_addr(COM2) + UART_CTLR_OFFSET)));

    TCB *cur = pop_ready_queue();

    int why = run_user(&cur->context);
    // printf(BW_COM2, "[Tid]: %d \t [Priority]: %d \r\n", cur->tid,
    //  cur->priority);

    int data = get_data(&cur->context);
    if (why == SYSCALL_CREATE) {
      create_args *args = (create_args *)data;
      int ret =
          scheduler_add(args->priority, args->function, cur->tid, args->name);
      // printf(BW_COM2, "[Ret]: %d \t [Func]: %d \r\n", ret,
      // args->function);
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
      // printf(BW_COM2, "[Exit] [Tid]: %d \t [Priority]: %d \r\n",
      // cur->tid,
      //        cur->priority);
      cur->state = ZOMBIE;
    } else if (why == SYSCALL_SHUTDOWN) {
      break;
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
        char debug_buffer[MAX_DEBUG_STRING_LEN];
        sprintf(debug_buffer, "[%s] Reply to [%s] EINVALIDTID", cur->task_name,
                target->task_name);
        debugprint(debug_buffer, CRITICAL_DEBUG);
      } else if (target->state != REPLY) {
        set_return(&cur->context, ENOTREPLYWAIT);
        add_to_ready_queue(cur);
        char debug_buffer[MAX_DEBUG_STRING_LEN];
        sprintf(debug_buffer, "[%s] Reply to [%s] ENOTREPLYWAIT",
                cur->task_name, target->task_name);
        debugprint(debug_buffer, CRITICAL_DEBUG);
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
      } else if (event == UART1_TX_INTR) {
        volatile int *uart1_mdmsts = (int *)(get_base_addr(COM1) + UART_MDMSTS_OFFSET);
        *uart1_mdmsts;
        enable_interrupt(UART1TXINTR);
        enable_interrupt(UART1CTSINTR);
        event_mapping[event] = cur;
        uart1_tx_flag = 0;
        interrupt_tasks++;
      } else if (event == UART1_RX_INTR) {
        enable_interrupt(UART1RXINTR);
        event_mapping[event] = cur;
        interrupt_tasks++;
      } else if (event == UART2_TX_HALF_EMPTY) {
        enable_interrupt(UART2TXINTR);
        event_mapping[event] = cur;
        interrupt_tasks++;
      } else if (event == UART2_RX_INCOMING) {
        // printf(BW_COM2, "A\r\n");
        enable_interrupt(UART2RXINTR);
        enable_interrupt(UART2RTIEINTR);
        event_mapping[event] = cur;
        interrupt_tasks++;
      } else if (event == TIMER_TICK) {
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

      // for uart 2 rx: buffer half full or timeout
      if (vic2_irq_status & VIC_INT_UART2_MASK) {

        int uart2_intr_combined =
            *(volatile int *)(UART2_BASE + UART_INTR_OFFSET);

        if ((uart2_intr_combined & RTIS_MASK) ||
            (uart2_intr_combined & RIS_MASK)) {
          // printf(BW_COM2, "B\r\n");
          if (wake_up(UART2_RX_INCOMING, event_mapping, &interrupt_tasks)) {
            disable_interrupt(UART2RXINTR);
            disable_interrupt(UART2RTIEINTR);
          } else {
            KASSERT(0, "Nobody home");
          }
        } else if (uart2_intr_combined & TIS_MASK) {
          if (wake_up(UART2_TX_HALF_EMPTY, event_mapping, &interrupt_tasks)) {
            // bw_uart_put_char(COM2, 'D');
            disable_interrupt(UART2TXINTR);
          } else {
            KASSERT(0, "Nobody home");
          }
        }
      }

      if (vic2_irq_status & VIC_INT_UART1_MASK) {
        volatile int *uart1_intr = (int *)(get_base_addr(COM1) + UART_INTR_OFFSET);
        volatile int *uart1_mdmsts = (int *)(get_base_addr(COM1) + UART_MDMSTS_OFFSET);
        if (*uart1_intr & RIS_MASK) {
          if (wake_up(UART1_RX_INTR, event_mapping, &interrupt_tasks)) {
            disable_interrupt(UART1RXINTR);
          } else {
            KASSERT(0, "Nobody home");
          }
        }
        if (*uart1_intr & TIS_MASK) {
          uart1_tx_flag |= 1;
          disable_interrupt(UART1TXINTR);
        }
        if (*uart1_intr & MIS_MASK) {
          *uart1_intr = 0;
          if ((*uart1_mdmsts & MSR_DCTS) && (*uart1_mdmsts & MSR_CTS)){
            uart1_tx_flag |= 2;
            disable_interrupt(UART1CTSINTR);
          }
        }

        if (uart1_tx_flag == 3){
           if (wake_up(UART1_TX_INTR, event_mapping, &interrupt_tasks)) {
            uart1_tx_flag = 0;
          } else {
            KASSERT(0, "Nobody home");
          }
        }
      }

      add_to_ready_queue(cur);
    } else {
      KASSERT(0, "Unknown Syscall\r\n");
    }
  }
  disable_irq();
  disable_cache();

  __asm__ volatile("MRS R0, CPSR\n\t"
                   "BIC R0, R0, #0x1F\n\t"
                   "ORR R0, R0, #0x12\n\r"
                   "MSR CPSR_c, R0\n\r"
                   "MOV SP, %[org_exc_stack]\n\r"
                   "ORR R0, R0, #1\n\r"
                   "MSR CPSR_c, R0\n\r" ::[org_exc_stack] "r"(org_exc_stack)
                   : "r0");

  *(volatile int *)0x28 = org_swi_vec;
  *(volatile int *)0x38 = org_irq_vec;
}
