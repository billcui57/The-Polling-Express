#include <user.h>

void task_k4_test();

void task_k4_init() {
  Create(20, nameserver);
  Create(10, uart_com2_tx_server);
  Create(10, uart_com1_server);
  Create(5, task_k4_test);
}

void task_k4_test() {
  int u1 = -1;
  while (u1 < 0)
    u1 = WhoIs("uart1");

  printf(COM2, "Starting..\r\n");

  Putc(u1, 0, '\x60');
  int data[10];
  while (true) {
    Putc(u1, 0, '\x85');
    for (int i = 0; i < 10; i++)
      data[i] = Getc(u1, 0);
    for (int module = 0; module < 5; module++) {
      int bmodule = module << 4;
      int res = data[2 * module] << 8 | data[2 * module + 1];
      for (int i = 0; i < 16; i++) {
        if (res & 1 << (15 - i)) {
          printf(COM2, "%c%d ", 'A' + module, i + 1);
        }
      }
    }
  }
  Putc(u1, 0, '\x61');
  printf(COM2, "Done\r\n");
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
      // printf(BW_COM2, ".");
    }
  }
}
