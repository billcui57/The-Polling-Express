#include <user.h>

void task_k3init() {
  Create(-10, task_k3_1);
  Create(-20, task_k3_2);
}

void task_k3_1() {
  for (;;) {
    AwaitEvent(TIMER_TICK);
    printf(&pc, "Tick\r\n");
  }
}

void task_k3_2() {
  for (;;) {
    printf(&pc, "Idle\r\n");
  }
}
