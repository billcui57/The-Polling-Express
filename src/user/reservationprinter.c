#include "reservationprinter.h"

void reservation_printer() {

  task_tid navigationserver_tid = WhoIsBlock("navigationserver");

  navigationserver_request req;
  navigationserver_response res;
  memset(&req, 0, sizeof(navigationserver_request));

  req.type = GET_RESERVATIONS;

  cursor_to_pos(RESERVATION_ROW, RESERVATION_COL, SUBSCRIBE_TABLE_WIDTH);
  printf(COM2, "[ Track Reservations ]\r\n");
  done_print();

  for (;;) {
    Send(navigationserver_tid, (char *)&req, sizeof(navigationserver_request),
         (char *)&res, sizeof(navigationserver_response));

    debugprint("Got back");

    cursor_to_pos(RESERVATION_ROW + 1, RESERVATION_COL, SUBSCRIBE_TABLE_WIDTH);
    printf(COM2, " ");

    for (int i = 0; i < TRACK_MAX; i++) {
      if (res.data.get_reservations.reservations[i] != -1) {
        printf(COM2, "[%s:%d]", track[i].name,
               v_p_train_num(res.data.get_reservations.reservations[i]));
      }
    }
    done_print();
  }
}