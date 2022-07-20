#include "shell.h"

void shell();
void fuckitprinter();
typedef enum {
  COMMAND_TR,
  COMMAND_SW,
  COMMAND_RV,
  COMMAND_Q,
  COMMAND_PF,
  COMMAND_REG
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
  cursor_to_pos(INPUT_ROW, INPUT_COL, INPUT_WIDTH);
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
  cursor_to_pos(LOG_ROW, LOG_COL, LOG_WIDTH);
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
  Create(5, "DebugPrinter", debugprinter);
#ifndef DEBUG_MODE
  Create(5, "TimerPrinter", timer_printer);
  Create(5, "SensorPrinter", sensor_printer);
#endif
  Create(5, "PathPrinter", path_printer);
  Create(5, "ReservationPrinter", reservation_printer);
  Create(5, "SwitchPrinter", switch_printer);
  Create(5, "SubscribePrinter", subscribe_printer);
  Create(6, "Shell", shell);
}

void print_art() {

  cursor_to_pos(ART_ROW, ART_COL, LINE_WIDTH);

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

void fuckitprinter() {
  for (;;) {
    printf(COM2, "hello\r");
    done_print();
  }
}

void shell() {
  clear_screen(COM2);

  hide_cursor();

  print_art();

  task_tid uart2_rx_tid = WhoIsBlock("uart2rxserver");

  task_tid trainserver_tid = WhoIsBlock("trainctl");

  task_tid timer_tid = WhoIsBlock("clockserver");

  task_tid navigation_server = WhoIsBlock("navigationserver");

  task_tid dispatchserver = WhoIsBlock("dispatchserver");

  char input[TERMINALMAXINPUTSIZE];
  memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);

  char *command_tokens[MAX_COMMAND_TOKENS];

  int prev_speed[MAX_NUM_TRAINS];
  memset(prev_speed, 0, sizeof(int) * MAX_NUM_TRAINS);

  v_train_num train_num;
  int speed;
  int switch_num;
  int switch_orientation;
  int dest_num;
  int source_num;
  int offset;

  int input_length = 0;

  char debug_buffer[100];

  print_input(input, &input_length);

  for (;;) {
    char c = Getc(uart2_rx_tid, IGNORE);
    print_debug("");
    bool entered = handle_new_char(c, input, &input_length, command_tokens);

    if (entered == true) {

      if (strncmp(command_tokens[0], "tr", strlen("tr")) == 0) {
        train_num = p_v_train_num(atoi(command_tokens[1]));

        speed = atoi(command_tokens[2]);

        if ((speed < 0) || (speed > 14)) {
          print_debug("Please enter a valid speed");
          continue;
        }

        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        sprintf(debug_buffer, "SPEED %d %d\r\n", atoi(command_tokens[1]),
                speed);
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
        train_num = p_v_train_num(atoi(command_tokens[1]));

        if (train_num < 0) {
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
        train_num = p_v_train_num(atoi(command_tokens[1]));
        speed = atoi(command_tokens[2]);
        dest_num = track_name_to_num(track, command_tokens[3]);
        offset = atoi(command_tokens[4]);

        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        if ((speed < 0) || (speed > 14)) {
          print_debug("Please enter a valid speed");
          continue;
        }

        if (dest_num < 0) {
          print_debug("Please enter a valid destination node");
          continue;
        }

        navigationserver_request req;
        navigationserver_response res;
        memset(&req, 0, sizeof(navigationserver_request));
        req.type = NAVIGATION_REQUEST;
        req.data.navigation_request.train = train_num;
        req.data.navigation_request.speed = speed;
        req.data.navigation_request.destination_num = dest_num;
        req.data.navigation_request.offset = offset;

        Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
             sizeof(res));

        if (res.type == NAVIGATIONSERVER_BUSY) {
          print_debug("Navigation server busy");
        } else if (res.type == NAVIGATIONSERVER_NO_PATH) {
          print_debug("No path");
        } else if (res.type == NAVIGATIONSERVER_NEED_REGISTER) {
          print_debug("Need to register train location first");
        } else {
          sprintf(debug_buffer, "Path Finding to %s + %d\r\n",
                  command_tokens[3], offset);
          print_debug(debug_buffer);
        }

      } else if (strncmp(command_tokens[0], "die", strlen("die")) == 0) {
        KASSERT(0, "DIE!");
      } else if (strncmp(command_tokens[0], "reg", strlen("reg")) == 0) {
        train_num = p_v_train_num(atoi(command_tokens[1]));
        source_num = track_name_to_num(track, command_tokens[2]);
        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        if (source_num < 0) {
          print_debug("Please enter a valid node");
          continue;
        }

        navigationserver_request req;
        navigationserver_response res;
        memset(&req, 0, sizeof(navigationserver_request));
        req.type = REGISTER_LOCATION;
        req.data.register_location.train_num = train_num;
        req.data.register_location.node_num = source_num;
        Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
             sizeof(res));

      } else if (strncmp(command_tokens[0], "mock", strlen("mock")) == 0) {

        int sensor_num = track_name_to_num(track, command_tokens[1]);

        sprintf(debug_buffer, "Mocking Sensor Trigger [%s] \r\n",
                track[sensor_num].name);
        print_debug(debug_buffer);

        int group = sensor_num / SENSORS_PER_GROUP;

        int sensor = sensor_num % SENSORS_PER_GROUP;

        dispatchserver_request req;
        dispatchserver_response res;
        memset(&req, 0, sizeof(dispatchserver_request));
        req.type = DISPATCHSERVER_SENSOR_UPDATE;
        req.data.sensor_update.time = Time(timer_tid);

        req.data.sensor_update.sensor_readings[group] = 0x80 >> sensor;

        Send(dispatchserver, (char *)&req, sizeof(dispatchserver_request),
             (char *)&res, sizeof(dispatchserver_response));
      }

      else {
        print_debug("Invalid Command Type");
      }
      memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
    }
  }
}
