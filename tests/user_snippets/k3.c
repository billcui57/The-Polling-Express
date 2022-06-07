#include <user.h>

#include <clockserver.h>
#include <nameserver.h>

#include <timer.h>

void task_test() {
  int clock_tid = -1;
  while (clock_tid < 0)
    clock_tid = WhoIs("clockserver");

  int start = Time(clock_tid);
  int i = 0;
  while (true) {
    int duc = DelayUntil(clock_tid, start + (i + 1) * 500);
    printf(&pc, "Tick start: %d, now: %d\r\n", duc, Time(clock_tid));
    i++;
  }
}

void task_k3init() {
  Create(-99, idle);
  Create(10, nameserver);
  Create(20, clockserver);

  // Create(5, task_test);
  // return;

  int a = Create(5, task_k3_client);
  int b = Create(4, task_k3_client);
  int c = Create(3, task_k3_client);
  int d = Create(2, task_k3_client);

  for (int i = 0; i < 4; i++) {
    int client;
    int junk = 0;
    Receive(&client, (char *)&junk, 0);
    int params[2];
    if (client == a) {
      params[0] = 10;
      params[1] = 20;
    } else if (client == b) {
      params[0] = 23;
      params[1] = 9;
    } else if (client == c) {
      params[0] = 33;
      params[1] = 6;
    } else if (client == d) {
      params[0] = 71;
      params[1] = 3;
    } else {
      KASSERT(0, "???");
    }
    Reply(client, (char *)&params, sizeof(params));
  }
}

void task_k3_client() {
  int params[2];
  int junk = 0;
  int tid = MyTid();
  Send(MyParentTid(), (char *)&junk, 0, (char *)&params, sizeof(params));
  int clock_tid = -1;
  while (clock_tid < 0)
    clock_tid = WhoIs("clockserver");
  for (int i = 0; i < params[1]; i++) {
    Delay(clock_tid, params[0]);
    printf(&pc, "TID: %d, Delay: %d, Completed: %d\r\n", tid, params[0], i + 1);
  }
}

void idle() {
  unsigned int start;

  read_timer(TIMER3, &start);

  unsigned int sleeping_time = 0;
  while (true) {
    unsigned int s = AwaitEvent(ANY_EVENT);
    sleeping_time += s;

    unsigned int now;
    read_timer(TIMER3, &now);
    unsigned int run = start - now;

    int percentage = (100 * sleeping_time) / run;
    printf(&pc, "Idle: %d%% (%d ms)\r\n", percentage,
           ticks_to_ms(sleeping_time, FASTCLOCKRATE));
  }
}
