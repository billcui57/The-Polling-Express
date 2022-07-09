#include "switchprinter.h"

void print_switch_table(char *branch_a, char *branch_b) {

  int row = 0;

  char table_name[] = "[ Switch States ]";
  cursor_to_pos(SWITCH_TABLE_ROW_BEGIN, SWITCH_TABLE_COL, strlen(table_name));
  printf(COM2, "%s\r\n", table_name);
  row++;
  for (int i = 1; i < 19; i++) {
    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + row, SWITCH_TABLE_COL,
                  SWITCH_TABLE_WIDTH);
    row++;
    char s = branch_a[i];
    printf(COM2, "[%d]:%c\r\n", i, "sc?"[s]);
  }
  for (int i = 0; i < 4; i++) {
    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + row, SWITCH_TABLE_COL,
                  SWITCH_TABLE_WIDTH);
    row++;
    char s = branch_b[i];
    printf(COM2, "[%d]:%c\r\n", 153 + i, "sc?"[s]);
  }
  done_print();
}

void switch_printer() {

  train_msg req;
  train_event event;
  memset(&req, 0, sizeof(req));
  req.type = BRANCH_EVENT;
  task_tid trainctl = WhoIsBlock("trainctl");

  for (int i = 1; i < 19; i++) {
    event.branch_a[i] = 2;
  }
  for (int i = 0; i < 4; i++) {
    event.branch_b[i] = 2;
  }
  print_switch_table(event.branch_a, event.branch_b);

  while (true) {
    Send(trainctl, (char *)&req, sizeof(req), (char *)&event,
         sizeof(train_event));

    print_switch_table(event.branch_a, event.branch_b);
  }
}