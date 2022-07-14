#include "pathprinter.h"

void path_printer() {

  task_tid navigationserver_tid = WhoIsBlock("navigationserver");

  navigationserver_request req;
  navigationserver_response res;
  memset(&req, 0, sizeof(navigationserver_request));

  req.type = GET_PATH_DISPLAY_INFO;

  cursor_to_pos(PATH_TABLE_ROW_BEGIN, PATH_TABLE_COL, PATH_TABLE_WIDTH);
  printf(COM2, "[ Path Destinations ]\r\n");
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    cursor_to_pos(PATH_TABLE_ROW_BEGIN + 1 + train_num, PATH_TABLE_COL,
                  PATH_TABLE_WIDTH);
    printf(COM2, "Train %d:", v_p_train_num(train_num));
  }
  done_print();

  for (;;) {
    Send(navigationserver_tid, (char *)&req, sizeof(navigationserver_request),
         (char *)&res, sizeof(navigationserver_response));

    int *dest_nums = res.data.get_path_display_info.dest_num;
    printf(COM2, " ");

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
      cursor_to_pos(PATH_TABLE_ROW_BEGIN + 1 + train_num, PATH_TABLE_COL,
                    PATH_TABLE_WIDTH);
      if (dest_nums[train_num] != -1) {
        printf(COM2, "Train %d: [%s]", v_p_train_num(train_num),
               track[dest_nums[train_num]].name);
      } else {
        printf(COM2, "Train %d:", v_p_train_num(train_num));
      }
    }

    done_print();
  }
}