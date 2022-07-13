#include "debugprinter.h"

circular_buffer *cb = NULL;
bool changed;

#define MAX_DEBUG_LINES 20

void debugprint(char *str) {
  if (cb == NULL) {
    return;
  }
  char new_str[MAX_DEBUG_STRING_LEN];
  strncpy(new_str, str, MAX_DEBUG_STRING_LEN);
  cb_push_back(cb, (void *)new_str, true);
  changed = true;
}

void debugprinter() {

  circular_buffer debug_cb;
  cb = &debug_cb;

  char debug_backing[MAX_DEBUG_LINES][MAX_DEBUG_STRING_LEN];
  for (int i = 0; i < MAX_DEBUG_LINES; i++) {
    memset(debug_backing[i], 0, sizeof(char) * MAX_DEBUG_STRING_LEN);
    cb_init(&debug_cb, debug_backing, MAX_DEBUG_LINES,
            sizeof(char) * MAX_DEBUG_STRING_LEN);
  }

  cursor_to_pos(DEBUG_TABLE_ROW_BEGIN, DEBUG_TABLE_COL, LINE_WIDTH);
  printf(COM2, "[ Debug Prints ]\r\n");
  done_print();

  for (;;) {
    if (!changed) {
      Yield();
    }

    int row = 0;

    while (!cb_is_empty(&debug_cb)) {
      char str[MAX_DEBUG_STRING_LEN];
      cb_pop_front(cb, (void *)str);
      cursor_to_pos(DEBUG_TABLE_ROW_BEGIN + row + 1, DEBUG_TABLE_COL,
                    LINE_WIDTH);
      printf(COM2, "%s\r\n", str);
      row++;
      done_print();
    }

    changed = false;
  }
}