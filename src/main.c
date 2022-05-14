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

  print_uart = &pc;

  assert_init(&pc);

  for (unsigned int i = 0; i < capacity; i++) {
    TCB *t = alloc_task(i, 'a' + i);
    stash[i] = t;
  }

  // for (unsigned int i = 0; i < capacity; i++) {
  //   free_task(stash[i]);
  // }

  for (unsigned int i = 0; i < capacity; i++) {
    printf("%c:%d,", heap[i]->name, heap[i]->priority);
  }

  printf("\r\n");

  remove_from_ready_queue(stash[3]);

  for (unsigned int i = 0; i < capacity; i++) {
    printf("%c:%d,", heap[i]->name, heap[i]->priority);
  }
  printf("\r\n");

  printf("No Crash!!! \r\n");
}
