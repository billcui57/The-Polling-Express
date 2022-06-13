#include <user.h>

void task_k4_test();

void task_k4_init() {
  Create(20, nameserver);
  Create(10, uart_com2_tx_server);

  for (int i = 0; i < 5; i++) {
    Create(5, task_k4_test);
  }
}

void task_k4_test() {
  int me = MyTid();

  for (;;) {
    printf(COM2, "tid: %d\r\n", me);
  }
}

void idle() {
  unsigned int start;

  read_timer(TIMER3, &start);

  unsigned int sleeping_time = 0;

  int i = 0;
  while (true) {
    unsigned int s = AwaitEvent(BREAK_IDLE);
    sleeping_time += s;
    i++;
    if (i % 10 == 0) {
      unsigned int now;
      read_timer(TIMER3, &now);
      unsigned int run = start - now;

      int percentage = (100 * sleeping_time) / run;
      printf(BW_COM2, "Idle: %d%% (%d ms)\r\n", percentage,
             ticks_to_ms(sleeping_time, FASTCLOCKRATE));
    }
  }
}
