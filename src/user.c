#include <user.h>

void task_k4_test();

void task_k4_init() {
  Create(10, nameserver);
  Create(5, uart_com2_server);
  Create(2, task_k4_test);
}

void task_k4_test() {
  task_tid uartserver_tid = WhoIs("uartserver");
  printf(COM2, "Uartserver tid: %d\r\n", uartserver_tid);
  for (;;) {
    Putc(uartserver_tid, 2, 'a');
    // printf(COM2, "Want to send: %d\r\n", status);
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
      // printf(COM2, "Idle: %d%% (%d ms)\r\n", percentage,
      //        ticks_to_ms(sleeping_time, FASTCLOCKRATE));
    }
  }
}
