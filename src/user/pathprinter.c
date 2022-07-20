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
    printf(COM2, "Train %d [Registered at ?]: [To ?]",
           v_p_train_num(train_num));
  }
  done_print();

  for (;;) {
    Send(navigationserver_tid, (char *)&req, sizeof(navigationserver_request),
         (char *)&res, sizeof(navigationserver_response));

    int *dest_nums = res.data.get_path_display_info.dest_num;
    int *src_nums = res.data.get_path_display_info.src_num;
    int *last_stopped_at = res.data.get_path_display_info.last_stopped_at;
    printf(COM2, " ");

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
      cursor_to_pos(PATH_TABLE_ROW_BEGIN + 1 + train_num, PATH_TABLE_COL,
                    PATH_TABLE_WIDTH);

      if (src_nums[train_num] != -1) {
        printf(COM2, "Train %d [Registered at %s]", v_p_train_num(train_num),
               track[src_nums[train_num]].name);
      } else {
        printf(COM2, "Train %d [Registered at ?]", v_p_train_num(train_num));
      }

      if (last_stopped_at[train_num] != -1) {
        printf(COM2, "[Last stopped at %s]: ",
               track[last_stopped_at[train_num]].name);
      } else {
        printf(COM2, "[Last stopped at ?]: ");
      }

      if (dest_nums[train_num] != -1) {
        printf(COM2, "[To %s]", track[dest_nums[train_num]].name);
      } else {
        printf(COM2, "[To ?]");
      }
    }

    done_print();
  }
}