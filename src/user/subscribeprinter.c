#include "subscribeprinter.h"

void subscribe_printer() {

  task_tid dispatchserver = WhoIsBlock("dispatchserver");

  dispatchserver_request req;
  memset(&req, 0, sizeof(dispatchserver_request));

  req.type = DISPATCHSERVER_SUBSCRIPTION_PRINT;

  dispatchserver_response res;

  cursor_to_pos(SUBSCRIBE_TABLE_ROW_BEGIN, SUBSCRIBE_TABLE_COL,
                SUBSCRIBE_TABLE_WIDTH);
  printf(COM2, "[ Sensor Subscriptions ]\r\n");
  done_print();

  int subscriptions_backing[MAX_NUM_TRAINS][MAX_SUBSCRIBED_SENSORS];
  circular_buffer subscriptions[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    cb_init(&(subscriptions[train_num]),
            (void *)subscriptions_backing[train_num], MAX_SUBSCRIBED_SENSORS,
            sizeof(int));
    cursor_to_pos(SUBSCRIBE_TABLE_ROW_BEGIN + train_num + 1,
                  SUBSCRIBE_TABLE_COL, SUBSCRIBE_TABLE_WIDTH);
    printf(COM2, "Train %d: ", v_p_train_num(train_num));
  }

  done_print();

  for (;;) {

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
      cb_clear(&(subscriptions[train_num]));
    }

    Send(dispatchserver, (char *)&req, sizeof(dispatchserver_request),
         (char *)&res, sizeof(dispatchserver_response));

    v_train_num *subscription_result =
        res.data.subscription_print.subscriptions;

    for (int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
      if (subscription_result[i] >= 0) {
        cb_push_back(&(subscriptions[subscription_result[i]]), (void *)&i,
                     true);
      }
    }

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {

      void *subscription_void[subscriptions[train_num].count];

      cb_to_array(&(subscriptions[train_num]), subscription_void);

      cursor_to_pos(SUBSCRIBE_TABLE_ROW_BEGIN + train_num + 1,
                    SUBSCRIBE_TABLE_COL + 10, SUBSCRIBE_TABLE_WIDTH);

      for (int i = 0; i < subscriptions[train_num].count; i++) {
        printf(COM2, "[%c%d]", (char)('A' + ((int)(subscription_void[i]) >> 4)),
               ((int)(subscription_void[i]) & 0xF) + 1);
      }
    }

    done_print();
  }
}