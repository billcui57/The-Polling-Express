#include <hal.h>
#include <syscall.h>
#include <user.h>

void task_test1() {
  while (1) {
    uart_put_str_block(&pc, "Hi from test1.\r\n");
    Yield();
  }
}
