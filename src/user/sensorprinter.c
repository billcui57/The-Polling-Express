#include "sensorprinter.h"

void sensor_printer() {

  task_tid dispatchhub = WhoIsBlock("dispatchhub");

  dispatchhub_request req;
  memset(&req, 0, sizeof(dispatchhub_request));

  req.type = DISPATCHHUB_SUBSCRIBE_SENSOR_PRINT;

  dispatchhub_response res;

  char table_name[] = "[ Sensor Triggers ]";

  cursor_to_pos(SENSOR_TABLE_ROW_BEGIN, SENSOR_TABLE_COL, strlen(table_name));
  printf(COM2, "%s\r\n", table_name);
  done_print();

  for (;;) {

    Send(dispatchhub, (char *)&req, sizeof(dispatchhub_request), (char *)&res,
         sizeof(dispatchhub_response));

    int pool_len = res.data.subscribe_sensor_print.sensor_pool_len;
    v_train_num *pool = res.data.subscribe_sensor_print.sensor_pool;
    unsigned int time = res.data.subscribe_sensor_print.time;

    int attributed_cols[MAX_NUM_TRAINS];
    int unattributed_col = 0;

    char name_buffer[20];
    memset(name_buffer, 0, sizeof(char) * 20);

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
      attributed_cols[train_num] = 0;
      cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + train_num + 1, SENSOR_TABLE_COL,
                    LINE_WIDTH);

      sprintf(name_buffer, "Train %d: ", v_p_train_num(train_num));

      printf(COM2, "%s", name_buffer);
      attributed_cols[train_num] += strlen(name_buffer);
    }
    memset(name_buffer, 0, sizeof(char) * 20);
    cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + MAX_NUM_TRAINS + 1, SENSOR_TABLE_COL,
                  LINE_WIDTH);
    sprintf(name_buffer, "Unattributed: ");
    printf(COM2, "%s", name_buffer);
    unattributed_col += strlen(name_buffer);

    for (int i = 0; i < pool_len; i++) {

      char sensor_print_buffer[20];

      memset(sensor_print_buffer, 0, sizeof(char) * 20);
      if (pool[i] >= 0) {
        cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + pool[i] + 1,
                      SENSOR_TABLE_COL + attributed_cols[pool[i]], 0);

        sprintf(sensor_print_buffer, "[%c%d]", (char)('A' + (i >> 4)),
                (int)((i & 0xF) + 1));

        printf(COM2, "%s\r\n", sensor_print_buffer);

        attributed_cols[pool[i]] += strlen(sensor_print_buffer);
      } else if (pool[i] == UNATTRIBUTED) {
        cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + MAX_NUM_TRAINS + 1,
                      SENSOR_TABLE_COL + unattributed_col, 0);

        sprintf(sensor_print_buffer, "[%c%d]", (char)('A' + (i >> 4)),
                (int)((i & 0xF) + 1));

        printf(COM2, "%s\r\n", sensor_print_buffer);

        unattributed_col += strlen(sensor_print_buffer);
      }
    }
    done_print();
  }
}