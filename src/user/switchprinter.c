#include "switchprinter.h"

void print_switch_table(char *branch_a, char *branch_b) {

  int col = 0;

  char table_name[] = "[ Switch States ]";
  cursor_to_pos(SWITCH_TABLE_ROW_BEGIN, SWITCH_TABLE_COL, strlen(table_name));
  printf(COM2, "%s\r\n", table_name);

  for (int i = 1; i < 10; i++) {
    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + 1, SWITCH_TABLE_COL + col,
                  SWITCH_TABLE_WIDTH);

    char s = branch_a[i];
    printf(COM2, "|00%d|\r\n", i);

    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + 2, SWITCH_TABLE_COL + col,
                  SWITCH_TABLE_WIDTH);

    printf(COM2, "| %c |\r\n", "sc?"[s]);
    col += 5;
  }
  for (int i = 10; i < 19; i++) {
    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + 1, SWITCH_TABLE_COL + col,
                  SWITCH_TABLE_WIDTH);

    char s = branch_a[i];
    printf(COM2, "|0%d|\r\n", i);

    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + 2, SWITCH_TABLE_COL + col,
                  SWITCH_TABLE_WIDTH);
    printf(COM2, "| %c |\r\n", "sc?"[s]);
    col += 5;
  }
  for (int i = 0; i < 4; i++) {
    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + 1, SWITCH_TABLE_COL + col,
                  SWITCH_TABLE_WIDTH);

    char s = branch_b[i];
    printf(COM2, "|%d|\r\n", 153 + i);
    cursor_to_pos(SWITCH_TABLE_ROW_BEGIN + 2, SWITCH_TABLE_COL + col,
                  SWITCH_TABLE_WIDTH);
    printf(COM2, "| %c |\r\n", "sc?"[s]);
    col += 5;
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