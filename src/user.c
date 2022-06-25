#include <user.h>

void task_k4_test();

void print_art();
void clear();
void shell();
void timer_printer();
void sensor_reader();

int idle_percentage;

void task_k4_init() {
  Create(20, nameserver);
  Create(10, clockserver);
  Create(10, uart_com2_tx_server);
  Create(10, uart_com1_server);
  Create(10, uart_com2_rx_server);
  Create(10, task_trainserver);
  Create(5, timer_printer);
  Create(5, sensor_reader);
  Create(5, shell);
}

#define LOG_ROW 30
#define INPUT_ROW LOG_ROW + 1
#define SENSOR_ROW 7
#define IDLE_ROW 5
#define TIME_ROW 6
#define SWITCH_TABLE_ROW_BEGIN 8

void print_art() {
  save_cursor();

#ifndef DEBUG_MODE
  printf(COM2, "\033[2J\033[H");
#endif

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

#ifndef DEBUG_MODE
  printf(COM2, "\033[%d;1H\033[K", SWITCH_TABLE_ROW_BEGIN);
#endif

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

#ifndef DEBUG_MODE
    printf(COM2, "\033[%d;1H\033[K", TIME_ROW);
#endif

    int formatted_time[3];
    memset(formatted_time, 0, sizeof(int) * 3);
    get_formatted_curr_time(Time(clock_tid), formatted_time, 100);
    printf(COM2, "Time: %d min %d.%d secs\r\n", formatted_time[0],
           formatted_time[1], formatted_time[2]);

#ifndef DEBUG_MODE
    printf(COM2, "\033[%d;1H\033[K", IDLE_ROW);
#endif

    printf(COM2, "Idle: %d%%\r\n", idle_percentage);
    restore_cursor();
    i++;
  }
}

#define SENSOR_READ_GROUPS 10
#define SENSOR_CB_BACK_CAPACITY 10

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

typedef enum {
  COMMAND_TR,
  COMMAND_SW,
  COMMAND_RV,
  COMMAND_Q,
  COMMAND_P
} command_type;

#define TERMINALMAXINPUTSIZE 20

bool is_num(char c) { return ('0' <= c) && (c <= '9'); }

void print_input(char *input, int *input_length) {
  save_cursor();
#ifndef DEBUG_MODE
  printf(COM2, "\033[%d;1H\033[K", INPUT_ROW);
#endif
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
#ifndef DEBUG_MODE
  printf(COM2, "\033[%d;1H\033[K", LOG_ROW);
#endif
  printf(COM2, "\033[32m");
  printf(COM2, "%s", input);
  printf(COM2, "\033[0m");
  printf(COM2, "\r\n");
  restore_cursor();
}

bool handle_new_char(char c, char *input, int *input_length,
                     int *parsed_command) { // backspace
  bool is_valid = false;

  if ((*input_length) == TERMINALMAXINPUTSIZE) {
    return is_valid;
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
    if (input[0] == 't' && input[1] == 'r') {
      int train_num = 0;
      int train_speed = 0;
      if (is_num(input[4])) // tr 23
      {
        train_num = (input[3] - '0') * 10 + (input[4] - '0');

        if (((*input_length) == 8) && is_num(input[7])) // tr 23 13
        {
          train_speed = (input[6] - '0') * 10 + (input[7] - '0');
        } else {
          train_speed = input[6] - '0';
        }
      } else {
        train_num = input[3] - '0';
        if (((*input_length) == 7) && is_num(input[6])) {
          train_speed = (input[5] - '0') * 10 + (input[6] - '0');
        } else {
          train_speed = input[5] - '0';
        }
      }
      parsed_command[0] = COMMAND_TR;
      parsed_command[1] = train_num;
      parsed_command[2] = train_speed;
      is_valid = true;
    } else if (input[0] == 's' && input[1] == 'w') {
      int switch_num = 0;
      char switch_dir = 0;
      if (is_num(input[5])) // sw 156
      {
        switch_num =
            (input[3] - '0') * 100 + (input[4] - '0') * 10 + input[5] - '0';

        switch_dir = input[7];
      } else if (is_num(input[4])) // sw 23
      {
        switch_num = (input[3] - '0') * 10 + (input[4] - '0');

        switch_dir = input[6];
      } else {
        switch_num = input[3] - '0';

        switch_dir = input[5];
      }
      parsed_command[0] = COMMAND_SW;
      parsed_command[1] = switch_num;
      parsed_command[2] = switch_dir;
      is_valid = true;
    } else if (input[0] == 'r' && input[1] == 'v') {
      int train_num = 0;
      if (is_num(input[4])) // tr 23
      {
        train_num = (input[3] - '0') * 10 + (input[4] - '0');
      } else {
        train_num = input[3] - '0';
      }
      parsed_command[0] = COMMAND_RV;
      parsed_command[1] = train_num;
      is_valid = true;
    } else if (input[0] == 'q') {
      parsed_command[0] = COMMAND_Q;
      is_valid = true;
    } else if (input[0] == 'p') {
      parsed_command[0] = COMMAND_P;
      is_valid = true;
    }
    memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
    (*input_length) = 0;
    print_input(input, input_length);
    return is_valid;
  } else {
    input[(*input_length)] = c;
    (*input_length)++;
    print_input(input, input_length);
  }

  // for (unsigned int i = 0; i < TERMINALMAXINPUTSIZE; i++) {
  //   printf(COM2, "%c", parsed_command[i] == '\0' ? 'x' : parsed_command[i]);
  // }

  return is_valid;
}

void hide_cursor() {
  save_cursor();
  printf(COM2, "\033[?25l");
  restore_cursor();
}

#define MAX_NUM_TRAINS 80

void shell() {

  hide_cursor();

  print_art();

  task_tid uart2_rx_tid = WhoIsBlock("uart2rxserver");

  task_tid trainserver_tid = WhoIsBlock("trainctl");

  task_tid timer_tid = WhoIsBlock("clockserver");

  char input[TERMINALMAXINPUTSIZE];
  memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
  int parsed_command[3];

  int prev_speed[MAX_NUM_TRAINS];
  memset(prev_speed, 0, sizeof(int) * MAX_NUM_TRAINS);

  char switch_state[NUM_SWITCHES];
  memset(switch_state, '?', sizeof(char) * NUM_SWITCHES);

  int train_num;
  int speed;
  int switch_num;
  int switch_orientation;

  int input_length = 0;

  char debug_buffer[100];

  print_input(input, &input_length);

  print_switch_table(switch_state);

  for (;;) {
    char c = Getc(uart2_rx_tid, IGNORE);
    bool got_valid_command =
        handle_new_char(c, input, &input_length, parsed_command);

    if (got_valid_command == true) {
      switch (parsed_command[0]) {
      case COMMAND_Q:
        save_cursor();
        print_debug("QUIT");
        Shutdown();
        break;
      case COMMAND_RV:
        train_num = parsed_command[1];

        if ((train_num < 1) || (train_num > MAX_NUM_TRAINS)) {
          print_debug("Please enter a valid train num");
          continue;
        }

        sprintf(debug_buffer, "REVERSE %d, back to speed %d\r\n", train_num,
                prev_speed[train_num]);
        print_debug(debug_buffer);

        TrainCommand(trainserver_tid, Time(timer_tid), REVERSE, train_num,
                     prev_speed[train_num]);
        break;
      case COMMAND_SW:
        switch_num = parsed_command[1];
        switch_orientation = parsed_command[2];

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
        break;
      case COMMAND_TR:

        train_num = parsed_command[1];
        speed = parsed_command[2];

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
        break;
      default:
        KASSERT(0, "INVALID COMMAND TYPE");
      }
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
