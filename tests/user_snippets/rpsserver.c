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