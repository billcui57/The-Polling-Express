void task_k2perf() {
  int sz[] = {4, 64, 256};
  int echo;
  int send;
  int a = 0;
  unsigned int timer;
  unsigned int time_diff;
  int cal = Create(100, task_calibrate);

  Send(cal, (char *)&a, 0, (char *)&timer, sizeof(unsigned int));
  printf(COM2, "Timer Calibration: %d\r\n", (timer * 1000) / 508);

  for (int i = 0; i < 3; i++) {
    echo = Create(10, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    time_diff -= timer;
    printf(COM2, "R %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
  for (int i = 0; i < 3; i++) {
    echo = Create(-5, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    time_diff -= timer;
    printf(COM2, "S %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
}

volatile unsigned int read_timer3() {
  return *(volatile unsigned int *)(TIMER3_BASE + VAL_OFFSET);
}

void task_calibrate() {
  int who;
  int a = 0;
  Receive(&who, (char *)&a, 0);
  unsigned int all = 0;
  for (int j = 0; j < 100; j++) {
    unsigned int start = read_timer3();
    for (int i = 0; i < 100; i++) {
      __asm__("nop");
    }
    unsigned int end = start - read_timer3();
    all += end;
  }
  all = all / 100;
  Reply(who, (char *)&all, sizeof(unsigned int));
}

void task_echo() {
  char buf[256];
  int who;
  for (int i = 0; i < 100; i++) {
    // printf(COM2, "Echo will call Receive\r\n");
    int len = Receive(&who, buf, 256);
    Reply(who, buf, len);
  }
}

void task_send() {
  int params[2];
  int who;
  Receive(&who, (char *)params, 2 * sizeof(int));
  int len = params[0];
  int echo = params[1];
  char buf[256];
  for (int i = 0; i < len; i++) {
    buf[i] = i & 0xF;
  }
  char rbuf[256];
  unsigned int start = read_timer3();
  for (int i = 0; i < 100; i++) {
    // printf(COM2, "Send will call Send\r\n");
    Send(echo, buf, len, rbuf, 256);
  }
  unsigned int end = start - read_timer3();
  Reply(who, (char *)&end, sizeof(unsigned int));
}
