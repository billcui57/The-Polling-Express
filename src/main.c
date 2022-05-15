#include <hal.h>
#include <task.h>

void kmain() {
  uart pc;
  uart_init(&pc, COM2);
  uart_put_str_block(&pc, "Hi\r\n");
  size_t capacity = 10;
  TCB backing[capacity];
  TCB *stash[capacity];
  memory_pool_init(capacity, backing);

  for (unsigned int i = 0; i < capacity; i++) {
    TCB *t = alloc_task();
    t->name = 'a' + i;
    t->val = i;
    stash[i] = t;
  }
  if (alloc_task())
    uart_put_str_block(&pc, "?\r\rn");
  for (unsigned int i = 0; i < capacity; i++) {
    free_task(stash[i]);
  }
  uart_put_str_block(&pc, "No Crash!!! \r\n");
}
