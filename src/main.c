#include <hal.h>
#include <kprintf.h>
#include <my_assert.h>
#include <task.h>

void kmain() {
  uart pc;
  uart_init(&pc, COM2);
  size_t capacity = 10;
  TCB backing[capacity];
  TCB *stash[capacity];
  memory_pool_init(capacity, backing);

  TCB *heap[capacity];
  scheduler_init(heap);

  assert_init(&pc);

  printf(&pc, "No Crash!!! \r\n");
}
