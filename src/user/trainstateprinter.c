#include "trainstateprinter.h"

#define NUM_CATEGORIES 4
void trainstate_printer() {

  task_tid navigationserver_tid = WhoIsBlock("navigationserver");

  navigationserver_request req;
  navigationserver_response res;
  memset(&req, 0, sizeof(navigationserver_request));

  req.type = GET_TRAIN_STATE;

  char train_num_str[] = "[ Train # |";
  char registered_at_str[] = "| Registered @ |";
  char dest_at_str[] = "| Dest @ |";
  char is_reversed_str[] = "| Reversed ? ]";
  char *categories[NUM_CATEGORIES] = {train_num_str, registered_at_str,
                                      dest_at_str, is_reversed_str};

  cursor_to_pos(PATH_TABLE_ROW_BEGIN, PATH_TABLE_COL, PATH_TABLE_WIDTH);

  for (int i = 0; i < NUM_CATEGORIES; i++) {
    char *category = categories[i];
    printf(COM2, "%s", category);
  }

  done_print();

  for (;;) {
    Send(navigationserver_tid, (char *)&req, sizeof(navigationserver_request),
         (char *)&res, sizeof(navigationserver_response));

    train_state_t *train_states = res.data.get_train_state.train_states;

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {

      cursor_to_pos(PATH_TABLE_ROW_BEGIN + train_num + 1, PATH_TABLE_COL,
                    PATH_TABLE_WIDTH);

      int col = PATH_TABLE_COL + 1;

      train_state_t train_state = train_states[train_num];
      cursor_to_pos(PATH_TABLE_ROW_BEGIN + train_num + 1, col, 0);
      printf(COM2, "%d", v_p_train_num(train_state.train_num));
      col += strlen(train_num_str);
      cursor_to_pos(PATH_TABLE_ROW_BEGIN + train_num + 1, col, 0);
      if (train_state.reged_at_num == -1) {
        printf(COM2, "?");
      } else {
        printf(COM2, "%s + %d", track[train_state.reged_at_num].name,
               train_state.reged_at_offset);
      }

      col += strlen(registered_at_str);
      cursor_to_pos(PATH_TABLE_ROW_BEGIN + train_num + 1, col, 0);
      if (train_state.dest_num == -1) {
        printf(COM2, "?");
      } else {
        printf(COM2, "%s + %d", track[train_state.dest_num].name,
               train_state.dest_offset);
      }
      col += strlen(dest_at_str);
      cursor_to_pos(PATH_TABLE_ROW_BEGIN + train_num + 1, col, 0);
      printf(COM2, "%s", train_state.reversed ? "ye" : "nah");
      done_print();
    }
  }
}