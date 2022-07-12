#include "sensorprinter.h"

void sensor_printer() {

  task_tid dispatchserver = WhoIsBlock("dispatchserver");

  dispatchserver_request req;
  memset(&req, 0, sizeof(dispatchserver_request));

  req.type = DISPATCHSERVER_SUBSCRIBE_SENSOR_PRINT;

  dispatchserver_response res;

  cursor_to_pos(SENSOR_TABLE_ROW_BEGIN, SENSOR_TABLE_COL, LINE_WIDTH);
  printf(COM2, "[ Sensor Triggers ]");
  done_print();

  typedef struct sensor_trigger {
    int sensor_num;
    int triggered_time;
  } sensor_trigger;

  sensor_trigger sensor_attributions_backing[MAX_NUM_TRAINS]
                                            [MAX_SUBSCRIBED_SENSORS];
  circular_buffer sensor_attributions[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    cb_init(&(sensor_attributions[train_num]),
            (void *)sensor_attributions_backing[train_num],
            MAX_SUBSCRIBED_SENSORS, sizeof(sensor_trigger));
    cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + train_num + 1, SENSOR_TABLE_COL,
                  LINE_WIDTH);
    printf(COM2, "Train %d: ", v_p_train_num(train_num));
  }

  sensor_trigger unattributed_sensors_backing[MAX_SUBSCRIBED_SENSORS];
  circular_buffer unattributed_sensors;
  cb_init(&unattributed_sensors, (void *)unattributed_sensors_backing,
          MAX_SUBSCRIBED_SENSORS, sizeof(sensor_trigger));
  cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + MAX_NUM_TRAINS + 1, SENSOR_TABLE_COL,
                LINE_WIDTH);
  printf(COM2, "Unattributed: ");

  done_print();

  for (;;) {

    Send(dispatchserver, (char *)&req, sizeof(dispatchserver_request),
         (char *)&res, sizeof(dispatchserver_response));

    v_train_num *pool = res.data.subscribe_sensor_print.sensor_pool;
    unsigned int time = res.data.subscribe_sensor_print.time;

    for (int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
      sensor_trigger st;
      st.sensor_num = i;
      st.triggered_time = time;
      if (pool[i] >= 0) {
        cb_push_back(&(sensor_attributions[pool[i]]), (void *)&st, true);
      } else if (pool[i] == UNATTRIBUTED) {
        cb_push_back(&unattributed_sensors, (void *)&st, true);
      }
    }

    for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {

      sensor_trigger attributed_sensors[sensor_attributions[train_num].count];

      cb_to_array(&(sensor_attributions[train_num]),
                  (void *)attributed_sensors);

      cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + train_num + 1,
                    SENSOR_TABLE_COL + 10, LINE_WIDTH);

      for (int i = 0; i < sensor_attributions[train_num].count; i++) {
        printf(COM2, "[Time %d|%c%d]", attributed_sensors[i].triggered_time,
               (char)('A' + ((int)(attributed_sensors[i].sensor_num) >> 4)),
               ((int)(attributed_sensors[i].sensor_num) & 0xF) + 1);
      }
    }

    sensor_trigger unattributed_sensors_arr[unattributed_sensors.count];

    cb_to_array(&unattributed_sensors, (void *)unattributed_sensors_arr);

    cursor_to_pos(SENSOR_TABLE_ROW_BEGIN + MAX_NUM_TRAINS + 1,
                  SENSOR_TABLE_COL + 15, LINE_WIDTH);
    for (int i = 0; i < unattributed_sensors.count; i++) {
      printf(COM2, "[Time %d|%c%d]", unattributed_sensors_arr[i].triggered_time,
             (char)('A' + ((int)(unattributed_sensors_arr[i].sensor_num) >> 4)),
             ((int)(unattributed_sensors_arr[i].sensor_num) & 0xF) + 1);
    }

    done_print();
  }
}