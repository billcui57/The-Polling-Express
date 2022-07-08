#include "shell.h"

void shell();
typedef enum {
  COMMAND_TR,
  COMMAND_SW,
  COMMAND_RV,
  COMMAND_Q,
  COMMAND_PF,
} command_type;

#define TERMINALMAXINPUTSIZE 20
#define MAX_COMMAND_TOKENS 6

#define NUM_SWITCHES 256

bool is_num(char c) { return ('0' <= c) && (c <= '9'); }

void hide_cursor() {
  printf(COM2, "\033[?25l");
  done_print();
}

void print_input(char *input, int *input_length) {
  cursor_to_row(INPUT_ROW);
  printf(COM2, "\033[35m");
  printf(COM2, ">");
  for (unsigned int i = 0; i < (*input_length); i++) {
    printf(COM2, "%c", input[i]);
  }
  printf(COM2, "_");
  printf(COM2, "\033[0m");
  printf(COM2, "\r\n");
  done_print();
}

void print_debug(char *input) {
  cursor_to_row(LOG_ROW);
  printf(COM2, "\033[32m");
  printf(COM2, "%s", input);
  printf(COM2, "\033[0m");
  printf(COM2, "\r\n");
  done_print();
}

int atoi(char *str) {
  int res = 0;

  bool negative = false;

  for (int i = 0; str[i] != '\0'; ++i) {
    if ((i == 0) && (str[i] == '-')) {
      negative = true;
      continue;
    }
    res = res * 10 + str[i] - '0';
  }

  if (negative) {
    return -res;
  }

  return res;
}

void tokenizer(char *input, char **tokens, unsigned int num_tokens) {

  unsigned int i = 0;

  tokens[i] = input;
  while (((*input) != '\00') && (i < num_tokens)) {

    if ((*input) == ' ') {
      *input = '\00';
      i++;
      input++;
      tokens[i] = input;
    } else {
      input++;
    }
  }
}

bool handle_new_char(char c, char *input, int *input_length,
                     char **command_tokens) { // backspace
  bool entered = false;

  if ((*input_length) == TERMINALMAXINPUTSIZE) {
    return entered;
  }

  if (c == '\b') {
    if ((*input_length) > 0) {
      input[(*input_length) - 1] = '\0';
      (*input_length) -= 1;
      print_input(input, input_length);
    }
  }

  // enter key
  else if (c == '\r') {

    tokenizer(input, command_tokens, MAX_COMMAND_TOKENS);

    entered = true;

    (*input_length) = 0;
    print_input(input, input_length);
    return entered;
  } else {
    input[(*input_length)] = c;
    (*input_length)++;
    print_input(input, input_length);
  }

  return entered;
}

void shell_init() {
#ifndef DEBUG_MODE
  Create(5, timer_printer);
#endif
  Create(5, switch_printer);
  Create(5, shell);
}

void print_art() {

  cursor_to_row(1);

  int christmas_colours[3] = {32, 37, 31};

  for (unsigned int i = 0; i < 35; i++) {
    printf(COM2, "\033[%dm=\033[0m", christmas_colours[i % 3]);
  }
  printf(COM2, "\r\n");

  printf(COM2, "|  The Polling Express (Track %c)  |\r\n", which_track);

  for (unsigned int i = 0; i < 35; i++) {
    printf(COM2, "\033[%dm=\033[0m", christmas_colours[i % 3]);
  }
  done_print();
}

void shell() {
  clear_screen(COM2);

  hide_cursor();

  print_art();

  task_tid uart2_rx_tid = WhoIsBlock("uart2rxserver");

  task_tid trainserver_tid = WhoIsBlock("trainctl");

  task_tid timer_tid = WhoIsBlock("clockserver");

  task_tid skynet_tid = WhoIsBlock("skynet");

  char input[TERMINALMAXINPUTSIZE];
  memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);

  char *command_tokens[MAX_COMMAND_TOKENS];

  int prev_speed[MAX_NUM_TRAINS];
  memset(prev_speed, 0, sizeof(int) * MAX_NUM_TRAINS);

  int train_num;
  int speed;
  int switch_num;
  int switch_orientation;
  char *dest_name;
  int offset;

  int input_length = 0;

  char debug_buffer[100];

  print_input(input, &input_length);

  for (;;) {
    char c = Getc(uart2_rx_tid, IGNORE);
    bool entered = handle_new_char(c, input, &input_length, command_tokens);

    if (entered == true) {

      if (strncmp(command_tokens[0], "tr", strlen("tr")) == 0) {
        train_num = atoi(command_tokens[1]);
        speed = atoi(command_tokens[2]);

        if ((speed < 0) || (speed > 14)) {
          print_debug("Please enter a valid speed");
          continue;
        }

        if ((train_num < 0) || (train_num > MAX_NUM_TRAINS)) {
          print_debug("Please enter a valid train num");
          continue;
        }

        sprintf(debug_buffer, "SPEED %d %d\r\n", train_num, speed);
        print_debug(debug_buffer);

        TrainCommand(trainserver_tid, Time(timer_tid), SPEED, train_num, speed);
        prev_speed[train_num] = speed;
      } else if (strncmp(command_tokens[0], "sw", strlen("sw")) == 0) {
        switch_num = atoi(command_tokens[1]);
        switch_orientation = command_tokens[2][0];

        if ((switch_num < 1) || (switch_num > NUM_SWITCHES)) {
          print_debug("Please enter a valid switch num");
          continue;
        }

        if ((switch_orientation != 'c') && (switch_orientation != 's')) {
          print_debug("Please enter a valid switch orientation");
          continue;
        }

        if ((switch_num < 1) || (switch_num > NUM_SWITCHES)) {
          print_debug("Please enter a valid switch num");
          continue;
        }

        if ((switch_orientation != 'c') && (switch_orientation != 's')) {
          print_debug("Please enter a valid switch orientation");
          continue;
        }

        sprintf(debug_buffer, "SWITCH %d %c\r\n", switch_num,
                switch_orientation);
        print_debug(debug_buffer);

        TrainCommand(trainserver_tid, Time(timer_tid), SWITCH, switch_num,
                     switch_orientation == 'c' ? 1 : 0);
      } else if (strncmp(command_tokens[0], "rv", strlen("rv")) == 0) {
        train_num = atoi(command_tokens[1]);

        if ((train_num < 0) || (train_num > MAX_NUM_TRAINS)) {
          print_debug("Please enter a valid train num");
          continue;
        }

        sprintf(debug_buffer, "REVERSE %d, back to speed %d\r\n", train_num,
                prev_speed[train_num]);
        print_debug(debug_buffer);

        TrainCommand(trainserver_tid, Time(timer_tid), REVERSE, train_num,
                     prev_speed[train_num]);
      } else if (strncmp(command_tokens[0], "q", strlen("q")) == 0) {

        print_debug("QUIT");
        Shutdown();

      } else if (strncmp(command_tokens[0], "gt", strlen("gt")) == 0) {
        skynet_msg req;
        memset(&req, 0, sizeof(req));
        req.type = SKYNET_TARGET;
        req.msg.target.train = atoi(command_tokens[1]);
        req.msg.target.speed = atoi(command_tokens[2]);
        req.msg.target.source = track_name_to_num(track, command_tokens[3]);
        req.msg.target.destination =
            track_name_to_num(track, command_tokens[4]);
        req.msg.target.offset = atoi(command_tokens[5]);
        // train_num = atoi(command_tokens[1]);
        // dest_name = command_tokens[2];
        // offset = atoi(command_tokens[3]);

        controlserver_response res;

        int status =
            Send(skynet_tid, (char *)&req, sizeof(skynet_msg), (char *)&res, 0);

        // sprintf(debug_buffer, "Path Finding Train %d to %s, offset %d \r\n",
        //         train_num, dest_name, offset);
        sprintf(debug_buffer, "Path Finding %s to %s + %d\r\n",
                command_tokens[3], command_tokens[4], req.msg.target.offset);
        print_debug(debug_buffer);

      } else if (strncmp(command_tokens[0], "die", strlen("die")) == 0) {
        KASSERT(0, "DIE!");
      } else {
        print_debug("Invalid Command Type");
      }
      memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
    }
  }
}