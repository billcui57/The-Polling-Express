#include "debugprinter.h"

void debugprint(char *str, int type) {

  if ((type & FILTER) == 0) {
    return;
  }

  char new_str[MAX_DEBUG_STRING_LEN];

  if (type == CRITICAL_DEBUG) {
    sprintf(new_str, "\033[31m%s\033[37m", str);
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

    sprintf(new_str, "[%s] %s", name_str, str);
  }

  cursor_to_pos(DEBUG_TABLE_ROW_BEGIN + DEBUG_TABLE_HEIGHT, DEBUG_TABLE_COL, 0);
  printf(COM2, "\033[D");
  printf(COM2, "%s\r\n", new_str);
  done_print();
}
