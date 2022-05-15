#include <context.h>
#include <hal.h>
#include <user.h>

registers kernel_reg;
user_task u;
uart pc;

void kmain() {
  uart_init(&pc, COM2);
  uart_put_str_block(&pc, "Kernel Init\r\n");
  *((void (**)())0x28) = &return_swi;
  __asm__ volatile("mov r9, %0" ::"irm"(&kernel_reg));
  init_user_task(&u, task_test1);
  while (true) {
    uart_put_str_block(&pc, "Kernel Loop\r\n");
    switch_user(&u.reg);
  }
  uart_put_str_block(&pc, "Exiting\r\n");
}
