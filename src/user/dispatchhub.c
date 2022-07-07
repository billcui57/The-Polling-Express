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

      void *sensor_attribution_backing[MAX_NUM_TRAINS]
                                      [NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      circular_buffer sensor_attribution[MAX_NUM_TRAINS];

      for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
        memset(sensor_attribution_backing[train_num], 0,
               sizeof(void *) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
        cb_init(&(sensor_attribution[train_num]),
                sensor_attribution_backing[train_num],
                NUM_SENSOR_GROUPS * SENSORS_PER_GROUP);
      }

      void *sensor_unattributed_backing[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      circular_buffer sensor_unattributed;

      memset(sensor_unattributed_backing, 0,
             sizeof(void *) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
      cb_init(&sensor_unattributed, sensor_unattributed_backing,
              NUM_SENSOR_GROUPS * SENSORS_PER_GROUP);

      void *sensor_pool_backing[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      circular_buffer sensor_pool;
      memset(sensor_pool_backing, 0,
             sizeof(void *) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
      cb_init(&sensor_pool, sensor_pool_backing,
              NUM_SENSOR_GROUPS * SENSORS_PER_GROUP);

      for (int i = 0; i < (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP); i++) {
        int a = i >> 3;
        int b = i & 7;
        if (sensor_readings[a] & 0x80 >> b) {

          if (!cb_is_empty(&(subscribers[i]))) {
            // unattributed
            cb_push_back(&sensor_unattributed, (void *)i, false);
          }

          while (!cb_is_empty(&(subscribers[i]))) {

            void *train_num_void = NULL;

            subscribers[i] = -1;

            cb_push_back(&(sensor_attribution[(v_train_num)train_num_void]),
                         (void *)i, false);
          }

          cb_push_back(&sensor_pool, (void *)i, false);
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
        res.data.subscribe_sensor_print.time = time;
        res.data.subscribe_sensor_print.unattributed_len =
            sensor_unattributed.count;
        cb_to_array(&sensor_unattributed,
                    res.data.subscribe_sensor_print.unattributed_sensors);
        res.data.subscribe_sensor_print.sensor_pool_len = sensor_pool.count;
        cb_to_array(&sensor_pool, res.data.subscribe_sensor_print.sensor_pool);

        for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS;
             train_num++) {

          res.data.subscribe_sensor_print.attributed_lens[train_num] =
              sensor_attribution[train_num].count;
          cb_to_array(
              &(sensor_attribution[train_num]),
              res.data.subscribe_sensor_print.attributed_sensors[train_num]);
        }
        Reply(sensor_printer, (char *)&res, sizeof(dispatchhub_response));
        sensor_printer = -1;
      }
    }
  }
}