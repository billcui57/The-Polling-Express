#include <kprintf.h>
#include <string.h>
#include <syscall.h>
#include <timer.h>
#include <user.h>

void task_k2rpsinit() {
  int ret;
  ret = Create(-10, nameserver);
  ret = Create(-10, rpsserver);

  Create(-10, task_k2rpsbot1);
  Create(-10, task_k2rpsbot2);
}

#define MAX_INPUT_LENGTH 100
#define MAX_OUTPUT_LENGTH 100

#define MAX_NUM_GAMES_BOT_PLAYS 50 // after this many games the bot will quit

void task_k2rpsbot1() {
  printf(&pc, "\033[36mPress any key to continue\033[0m\r\n");

  bw_uart_get_char(&pc);

  printf(&pc, "\033[2J");
  printf(&pc, "\033[H");

  int game_id;
  int status = SignUp(&game_id);

  for (int i = 0;; i++) {

    int move = ((i % 7) % 3) + 2;

    if (move == MOVE_PAPER) {
      printf(&pc, "[Bot 1 Match %d] Chose paper\r\n", i);

    } else if (move == MOVE_ROCK) {
      printf(&pc, "[Bot 1 Match %d] Chose rock\r\n", i);
    } else if (move == MOVE_SCISSOR) {
      printf(&pc, "[Bot 1 Match %d] Chose scissor\r\n", i);
    }

    int result = Play(game_id, move);

    if (result == RESPONSE_YOU_LOST) {
      printf(&pc, "[Bot 1 Match %d] lost\r\n", i);
    } else if (result == RESPONSE_TIE) {
      printf(&pc, "[Bot 1 Match %d] tied\r\n", i);
    } else if (result == RESPONSE_YOU_WON) {
      printf(&pc, "[Bot 1 Match %d] won\r\n", i);
    } else if (result == RESPONSE_GAME_ENDED) {
      printf(&pc,
             "[Bot 1 Match %d] The other player has left the game, press any "
             "key to quit\r\n",
             i);
      bw_uart_get_char(&pc);
      return;
    } else {
      printf(&pc, "Something terrible has happened\r\n");
    }
    printf(&pc, "\033[36mPress any key to continue\033[0m\r\n");
    bw_uart_get_char(&pc);
  }

  status = Quit(game_id);
  printf(&pc, "[Bot 1] Quit the game\r\n");
}

void task_k2rpsbot2() {

  int game_id;
  int status = SignUp(&game_id);

  for (int i = 0; i < MAX_NUM_GAMES_BOT_PLAYS; i++) {

    int move = ((i % 11) % 3) + 2;

    if (move == MOVE_PAPER) {
      printf(&pc, "[Bot 2 Match %d] Chose paper\r\n", i);
    } else if (move == MOVE_ROCK) {
      printf(&pc, "[Bot 2 Match %d] Chose rock\r\n", i);
    } else if (move == MOVE_SCISSOR) {
      printf(&pc, "[Bot 2 Match %d] Chose scissor\r\n", i);
    }

    int result = Play(game_id, move);

    if (result == RESPONSE_YOU_LOST) {
      printf(&pc, "[Bot 2 Match %d] lost\r\n", i);
    } else if (result == RESPONSE_TIE) {
      printf(&pc, "[Bot 2 Match %d] tied\r\n", i);
    } else if (result == RESPONSE_YOU_WON) {
      printf(&pc, "[Bot 2 Match %d] won\r\n", i);
    } else if (result == RESPONSE_GAME_ENDED) {
      printf(&pc,
             "[Bot 2 Match %d] The other player has left the game, press any "
             "key to quit\r\n",
             i);
      bw_uart_get_char(&pc);
      return;
    } else {
      printf(&pc, "Something terrible has happened\r\n");
    }

    int result = Play(game_id, move);
    printf(&pc, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\tReturning |%d|\r\n", result);
  }

  status = Quit(game_id);
  printf(&pc, "[Bot 2] Quit the game\r\n");
}

void task_k2perf() {
  int sz[] = {4, 64, 256};
  int echo;
  int send;
  int a = 0;
  unsigned int timer;
  unsigned int time_diff;
  int cal = Create(100, task_calibrate);

  Send(cal, (char *)&a, 0, (char *)&timer, sizeof(unsigned int));
  printf(&pc, "Timer Calibration: %d\r\n", (timer * 1000) / 508);

  for (int i = 0; i < 3; i++) {
    echo = Create(10, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    time_diff -= timer;
    printf(&pc, "noopt cache R %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
  for (int i = 0; i < 3; i++) {
    echo = Create(-5, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    time_diff -= timer;
    printf(&pc, "noopt cache S %d %d \r\n", sz[i], (time_diff * 10) / 508);
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
  for (int j = 0; j<100; j++){
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
  int parent = MyParentTid();
  printf(&pc, "Me: %d Parent: %d \r\n", me, parent);
  Yield();
  printf(&pc, "Me: %d Parent: %d \r\n", me, parent);
  Exit();
}
