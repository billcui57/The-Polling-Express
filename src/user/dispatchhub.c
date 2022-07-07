#include "dispatchhub.h"

void dispatchhub() {

  RegisterAs("dispatchhub");

  dispatchhub_request req;
  dispatchhub_response res;
  memset(&res, 0, sizeof(dispatchhub_response));

  task_tid client;

  v_train_num skynets[MAX_NUM_TRAINS];

  task_tid sensor_printer;

  for (v_train_num train_num; train_num < MAX_NUM_TRAINS; train_num++) {
    // skynets[train_num] = Create(blah blah blah)
  }

  v_train_num subscribers[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
  for (int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
    subscribers[i] = -1;
  }

  for (;;) {
    Receive(&client, (char *)&req, sizeof(dispatchhub_request));

    if (req.type == DISPATCHHUB_SUBSCRIBE_SENSOR_LIST) {

      int *subscribed_sensors =
          req.data.subscribe_sensor_list.subscribed_sensors;
      int subscribed_sensors_len = req.data.subscribe_sensor_list.len;

      KASSERT(subscribed_sensors_len <= MAX_SUBSCRIBED_SENSORS,
              "Can only subscribe to limited number of sensors");

      v_train_num train_num = req.data.subscribe_sensor_list.train_num;

      for (int i = 0; i < subscribed_sensors_len; i++) {
        KASSERT(subscribers[subscribed_sensors[i]] == -1,
                "Only one task can subscribe to a sensor at a time");
        subscribers[subscribed_sensors[i]] = train_num;
      }

    } else if (req.type == DISPATCHHUB_SUBSCRIBE_SENSOR_PRINT) {
      sensor_printer = client;
    } else if (req.type == DISPATCHHUB_SENSOR_UPDATE) {

      memset((void *)&res, 0, sizeof(dispatchhub_response));
      res.type = DISPATCHHUB_GOOD;
      Reply(client, (char *)&res, sizeof(dispatchhub_response));

      int *sensor_readings = req.data.sensor_update.sensor_readings;
      unsigned int time = req.data.sensor_update.time;

      void *sensor_attribution_backing[MAX_NUM_TRAINS][MAX_SUBSCRIBED_SENSORS];
      circular_buffer sensor_attribution[MAX_NUM_TRAINS];

      for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
        memset(sensor_attribution_backing[train_num], 0,
               sizeof(void *) * (MAX_SUBSCRIBED_SENSORS));
        cb_init(&(sensor_attribution[train_num]),
                sensor_attribution_backing[train_num], MAX_SUBSCRIBED_SENSORS);
      }

      v_train_num *sensor_pool[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];

      for (int i = 0; i < (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP); i++) {

        sensor_pool[i] = NOT_TRIGGERED;

        int a = i >> 3;
        int b = i & 7;
        if (sensor_readings[a] & 0x80 >> b) {

          if (subscribers[i] == -1) {
            // unattributed
            sensor_pool[i] = UNATTRIBUTED;
          } else {
            int status = cb_push_back(&(sensor_attribution[subscribers[i]]),
                                      (void *)i, false);
            KASSERT(status != CB_FULL,
                    "Attributed sensors len must be <= MAX_SUBSCRIBED_SENSORS");
            sensor_pool[i] = subscribers[i];
          }

          subscribers[i] = -1;
        }
      }

      for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {

        if (sensor_attribution[train_num].count == 0) {
          continue;
        }
        memset((void *)&res, 0, sizeof(dispatchhub_response));
        res.type = DISPATCHHUB_GOOD;
        res.data.subscribe_sensor_list.len =
            sensor_attribution[train_num].count;
        res.data.subscribe_sensor_list.time = time;
        cb_to_array(&(sensor_attribution[train_num]),
                    res.data.subscribe_sensor_list.triggered_sensors);
        Reply(skynets[train_num], (char *)&res, sizeof(dispatchhub_response));
      }

      if (sensor_printer != -1) {
        memset((void *)&res, 0, sizeof(dispatchhub_response));
        res.type = DISPATCHHUB_GOOD;
        res.data.subscribe_sensor_print.sensor_pool_len =
            NUM_SENSOR_GROUPS * SENSORS_PER_GROUP;
        res.data.subscribe_sensor_print.time = time;
        memcpy(res.data.subscribe_sensor_print.sensor_pool, sensor_pool,
               sizeof(v_train_num) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
        Reply(sensor_printer, (char *)&res, sizeof(dispatchhub_response));
        sensor_printer = -1;
      }
    }
  }
}