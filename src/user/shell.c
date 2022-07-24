#include "shell.h"

void shell();

#define TERMINALMAXINPUTSIZE 20
#define MAX_COMMAND_TOKENS 6
#define MAX_PLANNED_COMMANDS 10
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
  clear_screen(COM2);
  printf(COM2, "\033[%d;%dr", DEBUG_TABLE_ROW_BEGIN + 1,
         DEBUG_TABLE_ROW_BEGIN + DEBUG_TABLE_HEIGHT); // for scrolling debug
  cursor_to_pos(DEBUG_TABLE_ROW_BEGIN, DEBUG_TABLE_COL, DEBUG_TABLE_WIDTH);
  printf(COM2, "[ Debug Prints ]\r\n");
  done_print();

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
  printf(COM2, "|       By Edwin Z, Bill C        |\r\n");

  for (unsigned int i = 0; i < 35; i++) {
    printf(COM2, "\033[%dm=\033[0m", christmas_colours[i % 3]);
  }
  done_print();
}

typedef enum {
  COMMAND_TR,
  COMMAND_SW,
  COMMAND_RV,
  COMMAND_Q,
  COMMAND_GT,
  COMMAND_DIE,
  COMMAND_REG,
  COMMAND_MOCK
} command_type_t;

typedef struct command_t {
  command_type_t type;
  union {
    struct {
      v_train_num train_num;
      int speed;
    } tr;

    struct {
      int switch_num;
      char switch_orientation;
    } sw;

    struct {
      v_train_num train_num;
    } rv;

    struct {
      v_train_num train_num;
      int dest_num;
      int offset;
    } gt;

    struct {
      v_train_num train_num;
      int source_num;
    } reg;

    struct {
      int sensor_num;
    } mock;
  } data;
} command_t;

void shell() {

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

  circular_buffer command_cb;
  command_t command_backing[MAX_PLANNED_COMMANDS];
  cb_init(&command_cb, (void *)command_backing, MAX_PLANNED_COMMANDS,
          sizeof(command_t));

  bool is_planning_mode = false;

  int input_length = 0;

  char debug_buffer[100];

  char empty = '\0';

  print_input(input, &input_length);

  for (;;) {
    char c = Getc(uart2_rx_tid, IGNORE);
    print_debug("");
    for (int i = 0; i < MAX_COMMAND_TOKENS; i++)
      command_tokens[i] = &empty;
    bool entered = handle_new_char(c, input, &input_length, command_tokens);

    if (entered == true) {

      command_t command;
      memset((void *)&command, 0, sizeof(command_t));

      if (strncmp(command_tokens[0], "plan", strlen("plan")) == 0) {

        if (!is_planning_mode) {
          print_debug("In planning mode");
          is_planning_mode = true;
        } else {
          print_debug("Already in planning mode");
        }
        continue;

      } else if (strncmp(command_tokens[0], "doneplan", strlen("doneplan")) ==
                 0) {

        if (is_planning_mode) {
          print_debug("Exiting planning mode, flushing planned commands");
          is_planning_mode = false;
        } else {
          print_debug("Not in planning mode");
          continue;
        }
      } else if (strncmp(command_tokens[0], "tr", strlen("tr")) == 0) {
        v_train_num train_num = p_v_train_num(atoi(command_tokens[1]));

        int speed = atoi(command_tokens[2]);

        if ((speed < 0) || (speed > 14)) {
          print_debug("Please enter a valid speed");
          continue;
        }

        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        sprintf(debug_buffer, "SPEED %d %d\r\n", v_p_train_num(train_num),
                speed);
        print_debug(debug_buffer);

        command.type = COMMAND_TR;
        command.data.tr.train_num = train_num;
        command.data.tr.speed = speed;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "sw", strlen("sw")) == 0) {
        int switch_num = atoi(command_tokens[1]);
        char switch_orientation = command_tokens[2][0];

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

        command.type = COMMAND_SW;
        command.data.sw.switch_num = switch_num;
        command.data.sw.switch_orientation = switch_orientation;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "rv", strlen("rv")) == 0) {
        v_train_num train_num = p_v_train_num(atoi(command_tokens[1]));

        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        sprintf(debug_buffer, "REVERSE %d\r\n", v_p_train_num(train_num));
        print_debug(debug_buffer);

        command.type = COMMAND_RV;
        command.data.rv.train_num = train_num;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "q", strlen("q")) == 0) {

        print_debug("QUIT");
        command.type = COMMAND_Q;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "gt", strlen("gt")) == 0) {
        v_train_num train_num = p_v_train_num(atoi(command_tokens[1]));
        int dest_num = track_name_to_num(track, command_tokens[2]);
        int offset = atoi(command_tokens[3]);

        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        if (dest_num < 0) {
          print_debug("Please enter a valid destination node");
          continue;
        }

        sprintf(debug_buffer, "Train %d go to %s + %d\r\n",
                v_p_train_num(train_num), track[dest_num].name, offset);
        print_debug(debug_buffer);

        command.type = COMMAND_GT;
        command.data.gt.train_num = train_num;
        command.data.gt.dest_num = dest_num;
        command.data.gt.offset = offset;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "die", strlen("die")) == 0) {

        command.type = COMMAND_DIE;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "reg", strlen("reg")) == 0) {
        v_train_num train_num = p_v_train_num(atoi(command_tokens[1]));
        int source_num = track_name_to_num(track, command_tokens[2]);
        if (train_num < 0) {
          print_debug("Please enter a valid train num");
          continue;
        }

        if (source_num < 0) {
          print_debug("Please enter a valid node");
          continue;
        }

        sprintf(debug_buffer, "Reg train %d at %s \r\n",
                v_p_train_num(train_num), track[source_num].name);
        print_debug(debug_buffer);

        command.type = COMMAND_REG;
        command.data.reg.source_num = source_num;
        command.data.reg.train_num = train_num;
        cb_push_back(&command_cb, (void *)&command, false);

      } else if (strncmp(command_tokens[0], "mock", strlen("mock")) == 0) {

        int sensor_num = track_name_to_num(track, command_tokens[1]);

        sprintf(debug_buffer, "Mocking Sensor Trigger [%s] \r\n",
                track[sensor_num].name);
        print_debug(debug_buffer);

        command.type = COMMAND_MOCK;
        command.data.mock.sensor_num = sensor_num;
        cb_push_back(&command_cb, (void *)&command, false);

      } else {
        print_debug("Invalid Command Type");
      }

      if (!is_planning_mode) {
        v_train_num command_train_num;
        int command_speed;
        int command_switch_num;
        char command_switch_orientation;
        int command_offset;
        int command_dest_num;
        int command_src_num;
        int command_sensor_num;
        while (command_cb.count > 0) {
          command_t command;
          cb_pop_front(&command_cb, (void *)&command);

          if (command.type == COMMAND_TR) {
            command_train_num = command.data.tr.train_num;
            command_speed = command.data.tr.speed;

            TrainCommand(trainserver_tid, Time(timer_tid), SPEED,
                         command_train_num, command_speed);
            prev_speed[command_train_num] = command_speed;
          } else if (command.type == COMMAND_SW) {
            command_switch_num = command.data.sw.switch_num;
            command_switch_orientation = command.data.sw.switch_orientation;

            TrainCommand(trainserver_tid, Time(timer_tid), SWITCH,
                         command_switch_num,
                         command_switch_orientation == 'c' ? 1 : 0);
          } else if (command.type == COMMAND_RV) {
            command_train_num = command.data.rv.train_num;

            TrainCommand(trainserver_tid, Time(timer_tid), SPEED,
                         command_train_num, 15);
          } else if (command.type == COMMAND_Q) {

            Shutdown();
          } else if (command.type == COMMAND_GT) {
            command_train_num = command.data.gt.train_num;
            command_dest_num = command.data.gt.dest_num;
            command_offset = command.data.gt.offset;

            navigationserver_request req;
            navigationserver_response res;
            memset(&req, 0, sizeof(navigationserver_request));
            req.type = NAVIGATION_REQUEST;
            req.data.navigation_request.train = command_train_num;
            req.data.navigation_request.destination_num = command_dest_num;
            req.data.navigation_request.offset = command_offset;

            Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
                 sizeof(res));

            if (res.type == NAVIGATIONSERVER_BUSY) {
              print_debug("Navigation server busy");
            } else if (res.type == NAVIGATIONSERVER_NO_PATH) {
              print_debug("No path");
            } else if (res.type == NAVIGATIONSERVER_NEED_REGISTER) {
              print_debug("Need to register train location first");
            } else {
              sprintf(debug_buffer, "Train %d go to %s + %d\r\n",
                      v_p_train_num(command_train_num),
                      track[command_dest_num].name, command_offset);
              print_debug(debug_buffer);
            }
          } else if (command.type == COMMAND_DIE) {
            KASSERT(0, "DIE!");
          } else if (command.type == COMMAND_REG) {
            command_train_num = command.data.reg.train_num;
            command_src_num = command.data.reg.source_num;

            navigationserver_request req;
            navigationserver_response res;
            memset(&req, 0, sizeof(navigationserver_request));
            req.type = REGISTER_LOCATION;
            req.data.register_location.train_num = command_train_num;
            req.data.register_location.node_num = command_src_num;
            Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
                 sizeof(res));
          } else if (command.type == COMMAND_MOCK) {
            command_sensor_num = command.data.mock.sensor_num;

            int group = command_sensor_num / SENSORS_PER_GROUP;

            int sensor = command_sensor_num % SENSORS_PER_GROUP;

            dispatchserver_request req;
            dispatchserver_response res;
            memset(&req, 0, sizeof(dispatchserver_request));
            req.type = DISPATCHSERVER_SENSOR_UPDATE;
            req.data.sensor_update.time = Time(timer_tid);

            req.data.sensor_update.sensor_readings[group] = 0x80 >> sensor;

            Send(dispatchserver, (char *)&req, sizeof(dispatchserver_request),
                 (char *)&res, sizeof(dispatchserver_response));
          }
        }
      }

      memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
    }
  }
}
