#include <kprintf.h>
#include <string.h>
#include <syscall.h>
#include <timer.h>
#include <user.h>

void task_k1init() {
  int ret;
  ret = Create(-10, nameserver);
  ret = Create(-10, rpsserver);

  Create(-10, task1);
  Create(-10, task2);
}

void task1() {
  int game_id;
  int status = SignUp(&game_id);

  printf(&pc, "1: I am in game %d\r\n", game_id);

  for (int i = 0;; i++) {

    int move = (i % 3) + 2;
    printf(&pc, "1: Move: %d\r\n", move);

    int result = Play(game_id, move);

    if (result == RESPONSE_YOU_LOST) {
      printf(&pc, "1: I lost\r\n");
    } else if (result == RESPONSE_TIE) {
      printf(&pc, "1: I tied \r\n");
    } else if (result == RESPONSE_YOU_WON) {
      printf(&pc, "1: I won \r\n");
    } else if (result == RESPONSE_NOT_YOUR_GAME) {
      printf(&pc, "1: Not my game \r\n");
    } else if (result == RESPONSE_GAME_ENDED) {
      printf(&pc, "1: Other player quit \r\n");
      break;
    } else {
      printf(&pc, "1: Uh oh \r\n");
    }
  }

  // Create(-10, task2); <- uncomment for stress test
  // Create(-10, task1);
}

void task2() {
  int game_id;
  int status = SignUp(&game_id);

  for (int i = 0; i < 100; i++) {
    int move = ((i % 7) % 3) + 2;
    printf(&pc, "2: Move: %d\r\n", move);
    int result = Play(game_id, move);

    if (result == RESPONSE_YOU_LOST) {
      printf(&pc, "2: I lost\r\n");
    } else if (result == RESPONSE_TIE) {
      printf(&pc, "2: I tied \r\n");
    } else if (result == RESPONSE_YOU_WON) {
      printf(&pc, "2: I won \r\n");
    } else if (result == RESPONSE_NOT_YOUR_GAME) {
      printf(&pc, "2: Not my game \r\n");
    } else if (result == RESPONSE_GAME_ENDED) {
      printf(&pc, "2: Other player quit \r\n");
      break;
    } else {
      printf(&pc, "2: Uh oh \r\n");
    }
  }

  printf(&pc, "2: About to quit \r\n");
  status = Quit(game_id);

  if (status == RESPONSE_GOOD) {
    printf(&pc, "2: Successfully quit game \r\n");
  }

  // Create(-10, task1);
  // Create(-10, task2);
}

void task_k2perf() {
  int sz[] = {4, 64, 256};
  int echo;
  int send;
  unsigned int time_diff;
  for (int i = 0; i < 3; i++) {
    echo = Create(10, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    printf(&pc, "noopt cache R %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
  for (int i = 0; i < 3; i++) {
    echo = Create(-5, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    printf(&pc, "noopt cache S %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
}

unsigned int read_timer3() {
  return *(unsigned int *)(TIMER3_BASE + VAL_OFFSET);
}

void task_echo() {
  char buf[256];
  int who;
  for (int i = 0; i < 100; i++) {
    // printf(&pc, "Echo will call Receive\r\n");
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
    // printf(&pc, "Send will call Send\r\n");
    Send(echo, buf, len, rbuf, 256);
  }
  unsigned int end = start - read_timer3();
  Reply(who, (char *)&end, sizeof(unsigned int));
}
