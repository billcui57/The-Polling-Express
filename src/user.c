#include <kprintf.h>
#include <syscall.h>
#include <user.h>

void task_k1init() {
  int ret;
  ret = Create(-10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(-10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  printf(&pc, "FirstUserTask: exiting\r\n");
  Exit();
}

void task_k1test() {
  int me = MyTid();
  printf(&pc, "Task %d: running\r\n", me);
  int parent = MyParentTid();
  printf(&pc, "Task %d: yielding\r\n", me);
  Yield();
  printf(&pc, "Task %d: exiting\r\n", me);
  Exit();
}
