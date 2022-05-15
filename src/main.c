#include <context.h>
#include <hal.h>
#include <kprintf.h>
#include <my_assert.h>
#include <syscall.h>
#include <task.h>
#include <user.h>

registers kernel_reg;
uart pc;

void kmain() {
  uart_init(&pc, COM2);
  size_t capacity = 10;
  TCB backing[capacity];
  TCB *heap[capacity];
  scheduler_init(capacity, backing, heap);

  assert_init(&pc);

  *((void (**)())0x28) = &return_swi;
  __asm__ volatile("mov r9, %0" ::"irm"(&kernel_reg));

  scheduler_add(0, task_k1init, -1);

  while (!scheduler_empty()) {
    TCB *cur = pop_ready_queue();
    int data;
    int why = run_user(&cur->context.reg, &data);
    if (why == SYSCALL_CREATE) {
      create_args *args = (create_args *)data;
      int ret = scheduler_add(args->priority, args->function, cur->tid);
      set_return(&cur->context.reg, ret);
    } else if (why == SYSCALL_MYTID) {
      set_return(&cur->context.reg, cur->tid);
    } else if (why == SYSCALL_MYPARENTTID) {
      set_return(&cur->context.reg, cur->parentTid);
    } else if (why == SYSCALL_YIELD) {
      // nop
    } else if (why == SYSCALL_EXIT) {
      continue; // leak tcb
    } else {
      KASSERT(0, "Unknown Syscall\r\n");
    }
    add_to_ready_queue(cur);
  }
}
