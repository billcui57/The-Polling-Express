#include "debugprinter.h"

void debugprint(char *str) {
  char new_str[MAX_DEBUG_STRING_LEN];
  sprintf(new_str, "[%d]%s", debug_index, str);
  cb_push_back(debug_cb, (void *)new_str, true);
  debug_changed = true;
  debug_index++;
}

void debugprinter() {

  task_tid clock = WhoIsBlock("clockserver");

  cursor_to_pos(DEBUG_TABLE_ROW_BEGIN, DEBUG_TABLE_COL, LINE_WIDTH);
  printf(COM2, "[ Debug Prints ]\r\n");
  done_print();

  for (;;) {
    if (!debug_changed) {
      Delay(clock, 10);
      continue;
    }

    for (int i = 0; i < MAX_DEBUG_LINES; i++) {
      cursor_to_pos(DEBUG_TABLE_ROW_BEGIN + i + 1, DEBUG_TABLE_COL, LINE_WIDTH);
    }
    done_print();

    int row = 0;
    char debug[MAX_DEBUG_LINES][MAX_DEBUG_STRING_LEN];
    cb_to_array(debug_cb, debug);
    for (int i = 0; i < debug_cb->count; i++) {
      cursor_to_pos(DEBUG_TABLE_ROW_BEGIN + row + 1, DEBUG_TABLE_COL,
                    LINE_WIDTH);
      printf(COM2, "%s\r\n", debug[i]);
      row++;
      done_print();
    }

    debug_changed = false;
  }
}