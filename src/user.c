#include <kprintf.h>
#include <string.h>
#include <syscall.h>
#include <timer.h>
#include <user.h>

void task_k2rpsinit() {
  int ret;
  ret = Create(-10, nameserver);
  ret = Create(-10, rpsserver);

  Create(-10, task_k2rpsuser);
  Create(-10, task_k2rpsbot);
}

#define MAX_INPUT_LENGTH 100
#define MAX_OUTPUT_LENGTH 100

void get_input(char *input) {
  printf(&pc, "\033[7;1H");
  printf(&pc, "\033[K");

  int input_length = 0;

  char entered_char = 'a';
  while (entered_char != '\r' && input_length < MAX_INPUT_LENGTH) {
    entered_char = bw_uart_get_char(&pc);

    if (entered_char == '\b') {
      if (input_length > 0)
        input_length--;
    } else {
      printf(&pc, "\033[35m%c\033[0m", entered_char);
      input[input_length] = entered_char;
      input_length++;
    }

    printf(&pc, "\033[7;%dH", input_length + 1);
    printf(&pc, "\033[K");
  }
  input_length = 0;
}

void print_game_status(char *str) {
  printf(&pc, "\033[3;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

void print_bot1_status(char *str) {
  printf(&pc, "\033[3;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

void print_bot2_status(char *str) {
  printf(&pc, "\033[4;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

void print_prompt(char *str) {
  printf(&pc, "\033[6;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

void print_win_stats(char *str) {
  printf(&pc, "\033[3;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

void print_bot1_message(char *str) {
  printf(&pc, "\033[1;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

void print_bot2_message(char *str) {
  printf(&pc, "\033[2;1H");
  printf(&pc, "\033[K");
  printf(&pc, "%s", str);
}

int atoi(char *str) {
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i)
    res = res * 10 + str[i] - '0';
  return res;
}

#define MAX_NUM_GAMES_BOT_PLAYS 50 // after this many games the bot will quit

void task_k2rpsuser() {
  printf(&pc, "\033[36mPress any key to continue\033[0m\r\n");

  bw_uart_get_char(&pc);

  printf(&pc, "\033[2J");
  printf(&pc, "\033[H");

  int game_id;
  int status = SignUp(&game_id);

  for (int i = 0;; i++) {
    int move = ((i % 7) % 3) + 2;

    // printf(&pc, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\t111Playing %d\r\n", move);

    if (move == MOVE_PAPER) {
      print_bot1_message("[Bot 1] Chose paper");
    } else if (move == MOVE_ROCK) {
      print_bot1_message("[Bot 1] Chose rock");
    } else if (move == MOVE_SCISSOR) {
      print_bot1_message("[Bot 1] Chose scissor");
    }

    int result = Play(game_id, move);

    if (result == RESPONSE_YOU_LOST) {
      print_bot1_status("[Bot 2] won");
    } else if (result == RESPONSE_TIE) {
      print_bot1_status("[Bot 1 & 2] tied");
    } else if (result == RESPONSE_YOU_WON) {
      print_bot1_status("[Bot 1] won");
    } else if (result == RESPONSE_GAME_ENDED) {
      print_bot1_status(
          "The other player has left the game, press any key to quit");
      break;
    } else {
      print_bot1_status("Something terrible has happened");
    }

    // printf(&pc, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\t111Status |%d|\r\n",
    // result);

    if (result == RESPONSE_GAME_ENDED) {
      print_bot1_message("[Bot 1] The other player has left the game");
      break;
    }

    bw_uart_get_char(&pc);
  }

  status = Quit(game_id);
  print_bot1_message("[Bot 1] Quit the game");

  // printf(&pc, "\033[36mPress any key to continue\033[0m\r\n");

  // bw_uart_get_char(&pc);

  // printf(&pc, "\033[2J");
  // printf(&pc, "\033[H");

  // char input[MAX_INPUT_LENGTH];
  // char output[MAX_OUTPUT_LENGTH];
  // memset(input, 0, MAX_INPUT_LENGTH);
  // memset(input, 0, MAX_OUTPUT_LENGTH);

  // int game_id;

  // printf(&pc,
  // "================================================================"
  //             "========\r\n");
  // printf(&pc, "|     Welcome to Edwin and Bill's amazing rock paper
  // scissors
  // "
  //             "game     |\r\n");
  // printf(&pc,
  // "================================================================"
  //             "========\r\n");

  // int status = SignUp(&game_id);

  // int wins = 0;
  // int ties = 0;

  // for (int i = 0;; i++) {
  //   sprintf(output, "You are in game %d", game_id);
  //   print_game_status(output);

  //   sprintf(output, "Wins: %d, Losses: %d, Ties: %d", wins, i - wins -
  //   ties,
  //           ties);
  //   print_win_stats(output);

  //   print_prompt(
  //       "\033[36mChoose move? (R=rock|P=paper|S=scissor|Q=quit)\033[0m");

  //   get_input(input);

  //   // char move = bw_uart_get_char(&pc);
  //   int move;

  //   if (input[0] == 'R' || input[0] == 'r') {
  //     move = MOVE_ROCK;
  //   } else if (input[0] == 'P' || input[0] == 'p') {
  //     move = MOVE_PAPER;
  //   } else if (input[0] == 'S' || input[0] == 's') {
  //     move = MOVE_SCISSOR;
  //   } else if (input[0] == 'Q' || input[0] == 'q') {
  //     status = Quit(game_id);
  //     if (status == RESPONSE_GOOD) {
  //       print_status("Successfully left the game");
  //       return;
  //     }
  //   }

  //   int result = Play(game_id, move);

  //   if (result == RESPONSE_YOU_LOST) {
  //     print_status("You lost");
  //   } else if (result == RESPONSE_TIE) {
  //     print_status("You tied");
  //     ties++;
  //   } else if (result == RESPONSE_YOU_WON) {
  //     print_status("You won");
  //     wins++;
  //   } else if (result == RESPONSE_GAME_ENDED) {
  //     print_status("The other player has left the game, press any key to
  //     quit"); break;
  //   } else {
  //     print_status("Something terrible has happened");
  //   }
  // }

  // bw_uart_get_char(&pc);

  // printf(&pc, "\033[2J");
}

void task_k2rpsbot() {

  int game_id;
  int status = SignUp(&game_id);

  for (int i = 0; i < MAX_NUM_GAMES_BOT_PLAYS; i++) {
    int move = ((i % 11) % 3) + 2;

    printf(&pc, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\t111Playing %d\r\n", move);

    if (move == MOVE_PAPER) {
      print_bot2_message("[Bot 2] Chose paper");
    } else if (move == MOVE_ROCK) {
      print_bot2_message("[Bot 2] Chose rock");
    } else if (move == MOVE_SCISSOR) {
      print_bot2_message("[Bot 2] Chose scissor");
    }

    int result = Play(game_id, move);
    printf(&pc, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\tReturning |%d|\r\n", result);
  }

  status = Quit(game_id);
  print_bot2_message("[Bot 2] Quit the game");
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
