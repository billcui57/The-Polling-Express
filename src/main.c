#include <context.h>
#include <hal.h>
#include <kprintf.h>
#include <my_assert.h>
#include <task.h>
#include <user.h>

registers kernel_reg;
uart pc;

void kmain() {
  uart_init(&pc, COM2);
  printf(&pc, "Kernel Init\r\n");
  size_t capacity = 10;
  TCB backing[capacity];
  memory_pool_init(capacity, backing);

  TCB *heap[capacity];
  scheduler_init(heap);

  assert_init(&pc);

  *((void (**)())0x28) = &return_swi;
  __asm__ volatile("mov r9, %0" ::"irm"(&kernel_reg));

  user_task u;

  init_user_task(&u, task_test1);
  while (true) {
    printf(&pc, "Kernel Loop\r\n");
    switch_user(&u.reg);
  }

  printf(&pc, "Exiting \r\n");
}
