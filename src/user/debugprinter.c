#include "debugprinter.h"

void debugprint(char *str, int type) {

  if ((type & FILTER) == 0) {
    return;
  }

  char new_str[MAX_DEBUG_STRING_LEN];

  if (type == CRITICAL_DEBUG) {
    sprintf(new_str, "\033[31m[%d] %s\033[37m", debug_index, str);
  } else {

    int MAX_DEBUG_NAME_LEN = 20;
    char name_str[MAX_DEBUG_NAME_LEN];

    if (type == PATH_WORKER_DEBUG) {
      strncpy(name_str, "Path Worker", strlen(MAX_DEBUG_NAME_LEN));
    } else if (type == DISPATCH_SERVER_DEBUG) {
      strncpy(name_str, "Dispatch Server", strlen(MAX_DEBUG_NAME_LEN));
    } else if (type == NAVIGATION_SERVER_DEBUG) {
      strncpy(name_str, "Navigation Server", strlen(MAX_DEBUG_NAME_LEN));
    } else if (type == STRAIGHT_PATH_WORKER_DEBUG) {
      strncpy(name_str, "Straightpath Worker", strlen(MAX_DEBUG_NAME_LEN));
    } else if (type == TRAIN_SERVER_DEBUG) {
      strncpy(name_str, "Train Server", strlen(MAX_DEBUG_NAME_LEN));
    }

    sprintf(new_str, "[%d][%s] %s", debug_index, name_str, str);
  }

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
    char debug[MAX_DEBUG_LINES * MAX_DEBUG_STRING_LEN];
    int len = cb_to_array(debug_cb, debug);
    for (int i = 0; i < len; i++) {
      cursor_to_pos(DEBUG_TABLE_ROW_BEGIN + row + 1, DEBUG_TABLE_COL,
                    LINE_WIDTH);
      printf(COM2, "%s\r\n", &debug[MAX_DEBUG_STRING_LEN * i]);
      row++;
      done_print();
    }

    debug_changed = false;
  }
}