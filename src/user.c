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

#define INPUT_ROW 10
#define LOG_ROW 9
#define SENSOR_ROW 7
#define IDLE_ROW 5
#define TIME_ROW 6

void print_art() {
  save_cursor();

#ifndef DEBUG_MODE
  printf(COM2, "\033[2J\033[H");
#endif

  printf(COM2, "===================================\r\n|       The "
               "Polling Express       |\r\n|    By: Edwin Zhang, Bill Cui    "
               "|\r\n===================================\r\n");
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

    printf(COM2, "Time: %d\r\n", Time(clock_tid));

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

  void *backing[SENSOR_CB_BACK_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, backing, SENSOR_CB_BACK_CAPACITY);

  int start = Time(clock_tid);
  int i = 0;

  while (true) {
    int duc = DelayUntil(clock_tid, start + (i + 1) * 10);
    i++;

    // #ifdef DEBUG_MODE
    //     printf(COM2, "ENTERING\r\n");
    // #endif

    TrainSensor(trainserver_tid, sensor_group_readings);

    // #ifdef DEBUG_MODE
    //     printf(COM2, "LEAVING\r\n");
    // #endif

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

void print_input(char *input) {
  save_cursor();
#ifndef DEBUG_MODE
  printf(COM2, "\033[%d;1H\033[K", INPUT_ROW);
#endif

  printf(COM2, "%s\r\n", input);
  restore_cursor();
}

bool handle_new_char(char c, char *input, int *parsed_command) { // backspace
  bool is_valid = false;

  if (strlen(input) == TERMINALMAXINPUTSIZE) {
    return is_valid;
  }

  if (c == '\b') {

    if (strlen(input) > 0) {
      parsed_command[strlen(input) - 1] = '\0';
      print_input(input);
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

        if ((strlen(input) == 8) && is_num(input[7])) // tr 23 13
        {
          train_speed = (input[6] - '0') * 10 + (input[7] - '0');
        } else {
          train_speed = input[6] - '0';
        }
      } else {
        train_num = input[3] - '0';
        if ((strlen(input) == 7) && is_num(input[6])) {
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
      parsed_command[2] = switch_dir == 'c' ? 1 : 0;
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
    print_input(input);
    return is_valid;
  } else {
    input[strlen(input)] = c;
    print_input(input);
  }
  return is_valid;
}

#define MAX_NUM_TRAINS 80

void shell() {

  print_art();

  task_tid uart2_rx_tid = WhoIsBlock("uart2rxserver");

  task_tid trainserver_tid = WhoIsBlock("trainctl");

  task_tid timer_tid = WhoIsBlock("clockserver");

  char input[TERMINALMAXINPUTSIZE];
  memset(input, '\0', sizeof(char) * TERMINALMAXINPUTSIZE);
  int parsed_command[3];

  int prev_speed[MAX_NUM_TRAINS];
  memset(prev_speed, 0, sizeof(int) * MAX_NUM_TRAINS);

  int train_num;
  int speed;
  int switch_num;
  int switch_orientation;

  memset(prev_speed, 0, sizeof(char) * SENSOR_READ_GROUPS);

  for (;;) {
    char c = Getc(uart2_rx_tid, IGNORE);
    bool got_valid_command = handle_new_char(c, input, parsed_command);

    if (got_valid_command == true) {
      switch (parsed_command[0]) {
      case COMMAND_Q:
        save_cursor();
#ifndef DEBUG_MODE
        printf(COM2, "\033[%d;1H\033[K", LOG_ROW);
#endif
        printf(COM2, "QUIT\r\n");
        restore_cursor();
        break;
      case COMMAND_RV:
        train_num = parsed_command[1];
        save_cursor();
#ifndef DEBUG_MODE
        printf(COM2, "\033[%d;1H\033[K", LOG_ROW);
#endif
        printf(COM2, "REVERSE %d, back to speed %d\r\n", train_num,
               prev_speed[train_num]);
        restore_cursor();
        TrainCommand(trainserver_tid, Time(timer_tid), REVERSE, train_num,
                     prev_speed[train_num]);
        break;
      case COMMAND_SW:
        switch_num = parsed_command[1];
        switch_orientation = parsed_command[2];
        save_cursor();
#ifndef DEBUG_MODE
        printf(COM2, "\033[%d;1H\033[K", LOG_ROW);
#endif
        printf(COM2, "SWITCH %d %c\r\n", switch_num,
               switch_orientation == 1 ? 'c' : 's');
        restore_cursor();
        TrainCommand(trainserver_tid, Time(timer_tid), SWITCH, switch_num,
                     switch_orientation);
        break;
      case COMMAND_TR:
        save_cursor();
#ifndef DEBUG_MODE
        printf(COM2, "\033[%d;1H\033[K", LOG_ROW);
#endif
        printf(COM2, "SPEED %d %d\r\n", parsed_command[1], parsed_command[2]);
        restore_cursor();
        train_num = parsed_command[1];
        speed = parsed_command[2];
        TrainCommand(trainserver_tid, Time(timer_tid), SPEED, train_num, speed);
        prev_speed[train_num] = speed;
        break;
      default:
        KASSERT(0, "INVALID COMMAND TYPE");
      }
    }
  }
}

void task_k4_test() {
  int u1 = -1;
  while (u1 < 0)
    u1 = WhoIs("uart1");

  printf(COM2, "Starting..\r\n");

  Putc(u1, 0, '\x60');
  int data[10];
  while (true) {
    Putc(u1, 0, '\x85');
    for (int i = 0; i < 10; i++)
      data[i] = Getc(u1, 0);
    for (int module = 0; module < 5; module++) {
      int bmodule = module << 4;
      int res = data[2 * module] << 8 | data[2 * module + 1];
      for (int i = 0; i < 16; i++) {
        if (res & 1 << (15 - i)) {
          printf(COM2, "%c%d ", 'A' + module, i + 1);
        }
      }
    }
  }
  Putc(u1, 0, '\x61');
  printf(COM2, "Done\r\n");
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
