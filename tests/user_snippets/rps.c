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
  }

  status = Quit(game_id);
  printf(&pc, "[Bot 2] Quit the game\r\n");
}
