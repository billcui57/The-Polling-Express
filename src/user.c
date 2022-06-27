#include <user.h>

void task_k4_test();

void print_art();
void clear();
void shell();
void timer_printer();
void sensor_reader();

int idle_percentage;

void task_k4_init() {
  init_tracka(&track);

  Create(20, nameserver);
  Create(10, clockserver);
  Create(10, uart_com2_tx_server);
  Create(10, uart_com1_server);
  Create(10, uart_com2_rx_server);
  Create(10, task_trainserver);
  Create(10, pathfinder_server);
  // Create(10, task_skynet);
  Create(5, timer_printer);
  // Create(5, sensor_reader);
  Create(5, shell);
}

void print_art() {
  save_cursor();

  cursor_to_row(1);

  int christmas_colours[3] = {32, 37, 31};

  for (unsigned int i = 0; i < 35; i++) {
    printf(COM2, "\033[%dm=\033[0m", christmas_colours[i % 3]);
  }
  printf(COM2, "\r\n");

  printf(COM2, "|       The Polling Express       |\r\n");

  for (unsigned int i = 0; i < 35; i++) {
    printf(COM2, "\033[%dm=\033[0m", christmas_colours[i % 3]);
  }
  restore_cursor();
}

#define NUM_SWITCHES 256
void print_switch_table(char *switch_state) {
  save_cursor();

  int NUM_VALID_SWITCH_INDICES = 22;
  const int valid_switch_indices[] = {1,  2,  3,   4,   5,   6,  7,  8,
                                      9,  10, 11,  12,  13,  14, 15, 16,
                                      17, 18, 153, 154, 155, 156};

  cursor_to_row(SWITCH_TABLE_ROW_BEGIN);

  for (int i = 0; i < NUM_VALID_SWITCH_INDICES; i++) {

    int switch_index = valid_switch_indices[i];

    printf(COM2, "[%d]:%c\r\n", switch_index, switch_state[switch_index]);
  }

  restore_cursor();
}

void timer_printer() {
  int clock_tid = -1;
  while (clock_tid < 0)
    clock_tid = WhoIs("clockserver");

  int start = Time(clock_tid);
  int i = 0;
  while (true) {
    int duc = DelayUntil(clock_tid, start + (i + 1) * 10);
    save_cursor();

    cursor_to_row(TIME_ROW);

    int formatted_time[3];
    memset(formatted_time, 0, sizeof(int) * 3);
    get_formatted_curr_time(Time(clock_tid), formatted_time, 100);
    printf(COM2, "Time: %d min %d.%d secs\r\n", formatted_time[0],
           formatted_time[1], formatted_time[2]);

    cursor_to_row(IDLE_ROW);

    printf(COM2, "Idle: %d%%\r\n", idle_percentage);
    restore_cursor();
    i++;
  }
}

#define SENSOR_READ_GROUPS 10
#define SENSOR_CB_BACK_CAPACITY 10
/*
void sensor_reader() {
  task_tid trainserver_tid = WhoIsBlock("trainctl");
  task_tid clock_tid = WhoIsBlock("clockserver");

  char sensor_group_readings[SENSOR_READ_GROUPS];
  memset(sensor_group_readings, 0, sizeof(char) * SENSOR_READ_GROUPS);

  void *backing[SENSOR_CB_BACK_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, backing, SENSOR_CB_BACK_CAPACITY);

  int start = Time(clock_tid);
  int i = 0;

  while (true) {
    int duc = DelayUntil(clock_tid, start + (i + 1) * 10);
    i++;

    TrainSensor(trainserver_tid, sensor_group_readings);

    for (int module = 0; module < 5; module++) {
      int res = sensor_group_readings[2 * module] << 8 |
                sensor_group_readings[2 * module + 1];

      for (int i = 0; i < 16; i++) {
        if (res & 1 << (15 - i)) {
          cb_push_back(&cb, (void *)(('A' + module) << 8 | ((char)(i + 1))),
                       true);
        }
      }
    }
#ifdef DEBUG_MODE
    for (int i = 0; i < 10; i++) {
      printf(COM2, "%d\t", sensor_group_readings[i]);
    }
    printf(COM2, "\r\n");
#endif

    void **ptr = cb.tail;
    save_cursor();
#ifndef DEBUG_MODE
    printf(COM2, "\033[%d;1H\033[K", SENSOR_ROW);
#endif

    for (int i = 0; i < cb.count; i++) {

      void *sr_void = *ptr;
      int sr = (int)sr_void;

#ifndef DEBUG_MODE
      printf(COM2, "%c%d ", sr >> 8, sr & 0xFF);
#endif

      ptr++;

      if (ptr == cb.buffer_end) {
        ptr = cb.buffer;
      }
    }
    restore_cursor();
  }
}
*/

typedef enum {
  COMMAND_TR,
  COMMAND_SW,
  COMMAND_RV,
  COMMAND_Q,
  COMMAND_PF,
} command_type;

#define TERMINALMAXINPUTSIZE 20

bool is_num(char c) { return ('0' <= c) && (c <= '9'); }

void print_input(char *input, int *input_length) {
  save_cursor();

  cursor_to_row(INPUT_ROW);
  printf(COM2, "\033[35m");
  printf(COM2, ">");
  for (unsigned int i = 0; i < (*input_length); i++) {
    printf(COM2, "%c", input[i]);
  }

  printf(COM2, "_\r\n");
  printf(COM2, "\033[0m");
  restore_cursor();
}

void print_debug(char *input) {
  save_cursor();
  cursor_to_row(LOG_ROW);
  printf(COM2, "\033[32m");
  printf(COM2, "%s", input);
  printf(COM2, "\033[0m");
  printf(COM2, "\r\n");
  restore_cursor();
}

int atoi(char *str) {
  int res = 0;

  for (int i = 0; str[i] != '\0'; ++i)
    res = res * 10 + str[i] - '0';

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

#define MAX_COMMAND_TOKENS 5
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

  // for (unsigned int i = 0; i < TERMINALMAXINPUTSIZE; i++) {
  //   printf(COM2, "%c", command_tokens[i] == '\0' ? 'x' : command_tokens[i]);
  // }

  return entered;
}

void hide_cursor() {
  save_cursor();
  printf(COM2, "\033[?25l");
  restore_cursor();
}

#define MAX_NUM_TRAINS 80

void shell() {
  clear_screen();

  hide_cursor();

  print_art();

  task_tid uart2_rx_tid = WhoIsBlock("uart2rxserver");

  task_tid trainserver_tid = WhoIsBlock("trainctl");

  task_tid timer_tid = WhoIsBlock("clockserver");

  task_tid pathfinder_tid = WhoIsBlock("pathfinderserver");

  char input[TERMINALMAXINPUTSIZE];
  memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);

  char *command_tokens[MAX_COMMAND_TOKENS];

  int prev_speed[MAX_NUM_TRAINS];
  memset(prev_speed, 0, sizeof(int) * MAX_NUM_TRAINS);

  char switch_state[NUM_SWITCHES];
  memset(switch_state, '?', sizeof(char) * NUM_SWITCHES);

  int train_num;
  int speed;
  int switch_num;
  int switch_orientation;
  char *dest_name;
  int offset;

  int input_length = 0;

  char debug_buffer[100];

  print_input(input, &input_length);

  print_switch_table(switch_state);

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

        if ((train_num < 1) || (train_num > MAX_NUM_TRAINS)) {
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

        sprintf(debug_buffer, "SWITCH %d %c\r\n", switch_num,
                switch_orientation);
        print_debug(debug_buffer);

        switch_state[switch_num] = switch_orientation;
        print_switch_table(switch_state);
        TrainCommand(trainserver_tid, Time(timer_tid), SWITCH, switch_num,
                     switch_orientation == 'c' ? 1 : 0);
      } else if (strncmp(command_tokens[0], "rv", strlen("rv")) == 0) {
        train_num = atoi(command_tokens[1]);

        if ((train_num < 1) || (train_num > MAX_NUM_TRAINS)) {
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

      } else if (strncmp(command_tokens[0], "pf", strlen("pf")) == 0) {
        pathfinderserver_request req;
        memset(&req, 0, sizeof(req));

        train_num = atoi(command_tokens[1]);
        dest_name = command_tokens[2];
        offset = atoi(command_tokens[3]);

        char *src_name = "C15";

        memcpy(req.src_name, src_name, strlen(src_name));
        memcpy(req.dest_name, dest_name, strlen(dest_name));
        pathfinderserver_response res;

        int status =
            Send(pathfinder_tid, (char *)&req, sizeof(pathfinderserver_request),
                 (char *)&res, sizeof(pathfinderserver_response));

        sprintf(
            debug_buffer,
            "Path Finding Train %d to %s, offset %d [Result next node: %d]\r\n",
            train_num, dest_name, offset, res.next_step_num);
        print_debug(debug_buffer);

      } else {
        print_debug("Invalid Command Type");
      }
      memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
    }
  }
}

void idle() {
  unsigned int start;

  read_timer(TIMER3, &start);

  unsigned long long sleeping_time = 0;

  int i = 0;
  while (true) {
    unsigned int s = AwaitEvent(BREAK_IDLE);
    sleeping_time += s;
    i++;
    if (i % 10 == 0) {
      unsigned int now;
      read_timer(TIMER3, &now);
      unsigned int run = start - now;

      idle_percentage = (100 * sleeping_time) / run;
    }
  }
}
