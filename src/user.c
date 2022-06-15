#include <user.h>

void task_k4_test();
void uart_2_test();

void print_art();
void clear();
void shell();
void timer_printer();

int idle_percentage;

void task_k4_init() {
  Create(20, nameserver);
  Create(10, clockserver);
  Create(10, uart_com2_tx_server);
  Create(10, uart_com1_server);
  Create(10, uart_com2_rx_server);
  Create(5, timer_printer);
  Create(5, shell);
}

void print_art() {
  printf(COM2,
         "\033[2J\033[H===================================\r\n|       The "
         "Polling Express       |\r\n|    By: Edwin Zhang, Bill Cui    "
         "|\r\n===================================");
}

void print_idle() {
  printf(COM2, "\033[5;1H\033[KIdle: %d%%", idle_percentage);
}

void timer_printer() {
  int clock_tid = -1;
  while (clock_tid < 0)
    clock_tid = WhoIs("clockserver");

  int start = Time(clock_tid);
  int i = 0;
  while (true) {
    int duc = DelayUntil(clock_tid, start + (i + 1) * 10);
    printf(COM2, "\033[6;1H\033[KTime: %d\r\n", Time(clock_tid));
    print_idle();
    i++;
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

#define INPUT_ROW 10

#define LOG_ROW 9

bool is_num(char c) { return ('0' <= c) && (c <= '9'); }

bool handle_new_char(char c, char *input, unsigned int *input_length,
                     int *parsed_command) { // backspace
  bool is_valid = false;

  if (*input_length == TERMINALMAXINPUTSIZE) {
    return is_valid;
  }

  if (c == '\b') {
    if (*input_length > 0) {
      *input_length -= 1;
      printf(COM2, "\033[%d;%dH\033[K", INPUT_ROW, *input_length + 1);
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

        if ((*input_length == 8) && is_num(input[7])) // tr 23 13
        {
          train_speed = (input[6] - '0') * 10 + (input[7] - '0');
        } else {
          train_speed = input[6] - '0';
        }
      } else {
        train_num = input[3] - '0';
        if ((*input_length == 7) && is_num(input[6])) {
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
      parsed_command[2] = (int)switch_dir;
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
    for (int i = 0; i < TERMINALMAXINPUTSIZE; i++) {
      input[i] = '\0';
    }
    printf(COM2, "\033[%d;1H\033[K", INPUT_ROW);
    *input_length = 0;
    return is_valid;
  } else {
    input[*input_length] = c;
    printf(COM2, "\033[%d;%dH%c", INPUT_ROW, *input_length + 1, c); // 1 indexed
    *input_length++;
  }
  return is_valid;
}

void shell() {

  print_art();

  task_tid uart2_rx_tid = -1;
  while (uart2_rx_tid < 0)
    uart2_rx_tid = WhoIs("uart2rxserver");

  char input[TERMINALMAXINPUTSIZE];
  memset(input, 0, sizeof(char) * TERMINALMAXINPUTSIZE);
  unsigned int input_length = 0;
  int parsed_command;

  for (;;) {
    char c = Getc(uart2_rx_tid, IGNORE);
    printf(COM2, "%c", c);
    // bool got_valid_command =
    //     handle_new_char(c, input, &input_length, &parsed_command);

    // if (got_valid_command == true) {
    //   // switch (parsed_command) {
    //   // case COMMAND_Q:
    //   //   printf(COM2, "\033[%d;1H\033[KQUIT", LOG_ROW);
    //   //   break;
    //   // case COMMAND_RV:
    //   //   printf(COM2, "\033[%d;1H\033[KREVERSE", LOG_ROW);
    //   //   break;
    //   // case COMMAND_SW:
    //   //   printf(COM2, "\033[%d;1H\033[KSWITCH", LOG_ROW);
    //   //   break;
    //   // case COMMAND_TR:
    //   //   printf(COM2, "\033[%d;1H\033[KSPEED", LOG_ROW);
    //   //   break;
    //   // default:
    //   //   KASSERT(0, "INVALID COMMAND TYPE");
    //   // }
    // }
  }
}

void uart_2_test() {
  int u1rx = -1;
  while (u1rx < 0) {
    u1rx = WhoIs("uart2rxserver");
  }

  for (;;) {
    // bw_uart_put_char(COM2, 'D');
    char c = Getc(u1rx, IGNORE);
    // bw_uart_put_char(COM2, 'E');
    printf(COM2, "%c", c);
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
