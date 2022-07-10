#include "sensorprinter.h"

void sensor_printer() {

  task_tid dispatchhub = WhoIsBlock("dispatchhub");

  dispatchhub_request req;
  memset(&req, 0, sizeof(dispatchhub_request));

  req.type = DISPATCHHUB_SUBSCRIBE_SENSOR_PRINT;

  dispatchhub_response res;

  cursor_to_pos(SENSOR_TABLE_ROW_BEGIN, SENSOR_TABLE_COL, LINE_WIDTH);
  printf(COM2, "[ Sensor Triggers ]");
  done_print();

  int sensor_attributions_backing[MAX_NUM_TRAINS][MAX_SUBSCRIBED_SENSORS];
  circular_buffer sensor_attributions[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    cb_init(&(sensor_attributions[train_num]),
            (void *)sensor_attributions_backing[train_num],
            MAX_SUBSCRIBED_SENSORS, sizeof(int));
    cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + train_num + 1, SENSOR_TABLE_COL,
                  LINE_WIDTH);
    printf(COM2, "Train %d: ", v_p_train_num(train_num));
  }

  int unattributed_sensors_backing[MAX_SUBSCRIBED_SENSORS];
  circular_buffer unattributed_sensors;
  cb_init(&unattributed_sensors, (void *)unattributed_sensors_backing,
          MAX_SUBSCRIBED_SENSORS, sizeof(int));
  cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + MAX_NUM_TRAINS + 1, SENSOR_TABLE_COL,
                LINE_WIDTH);
  printf(COM2, "Unattributed: ");

  done_print();

  for (;;) {

    Send(dispatchhub, (char *)&req, sizeof(dispatchhub_request), (char *)&res,
         sizeof(dispatchhub_response));

    v_train_num *pool = res.data.subscribe_sensor_print.sensor_pool;
    unsigned int time = res.data.subscribe_sensor_print.time;

    for (int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
      if (pool[i] >= 0) {
        cb_push_back(&(sensor_attributions[pool[i]]), (void *)&i, true);
      } else if (pool[i] == UNATTRIBUTED) {
        cb_push_back(&unattributed_sensors, (void *)&i, true);
      }
    }

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {

      void *attributed_sensors[sensor_attributions[train_num].count];

      cb_to_array(&(sensor_attributions[train_num]), attributed_sensors);

      cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + train_num + 1,
                    SENSOR_TABLE_COL + 10, LINE_WIDTH);

      for (int i = 0; i < sensor_attributions[train_num].count; i++) {
        printf(COM2, "[Time %d|%c%d]", time,
               (char)('A' + ((int)(attributed_sensors[i]) >> 4)),
               ((int)(attributed_sensors[i]) & 0xF) + 1);
      }
    }

    void *unattributed_sensors_arr[unattributed_sensors.count];

    cb_to_array(&unattributed_sensors, unattributed_sensors_arr);

    cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + MAX_NUM_TRAINS + 1,
                  SENSOR_TABLE_COL + 15, LINE_WIDTH);
    for (int i = 0; i < unattributed_sensors.count; i++) {
      printf(COM2, "[Time %d|%c%d]", time,
             (char)('A' + ((int)(unattributed_sensors_arr[i]) >> 4)),
             ((int)(unattributed_sensors_arr[i]) & 0xF) + 1);
    }

    done_print();
  }
}