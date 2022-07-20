#include "dispatchserver.h"
#include "straightpathworker.h"

void inform_subscriber(int subscribe_printer, v_train_num *subscribers) {
  debugprint("Informing subscribe printer", 10);
  dispatchserver_response res;
  memset(&res, 0, sizeof(dispatchserver_response));
  res.type = DISPATCHSERVER_GOOD;
  memcpy(res.data.subscription_print.subscriptions, subscribers,
         sizeof(v_train_num) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
  Reply(subscribe_printer, (char *)&res, sizeof(dispatchserver_response));
}

void dispatchserver() {

  RegisterAs("dispatchserver");

  Create(10, "SensorCourier", sensor_courier);

  dispatchserver_request req;
  dispatchserver_response res;
  memset(&res, 0, sizeof(dispatchserver_response));

  task_tid client;

  task_tid straightpathworkers[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    straightpathworkers[train_num] = -1;
  }

  task_tid sensor_printer = -1;
  task_tid subscribe_printer = -1;

  v_train_num subscribers[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
  for (int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
    subscribers[i] = -1;
  }

  bool subscription_changed = false;

  for (;;) {
    Receive(&client, (char *)&req, sizeof(dispatchserver_request));

    if (req.type == DISPATCHSERVER_SUBSCRIBE_SENSOR_LIST) {

      int *subscribed_sensors =
          req.data.subscribe_sensor_list.subscribed_sensors;
      int subscribed_sensors_len = req.data.subscribe_sensor_list.len;

      KASSERT(subscribed_sensors_len <= MAX_SUBSCRIBED_SENSORS,
              "Can only subscribe to limited number of sensors");

      v_train_num train_num = req.data.subscribe_sensor_list.train_num;

      straightpathworkers[train_num] = client;

      for (int i = 0; i < subscribed_sensors_len; i++) {
        KASSERT(subscribers[subscribed_sensors[i]] == -1,
                "Only one task can subscribe to a sensor at a time");
        subscribers[subscribed_sensors[i]] = train_num;

        char debug_buffer[MAX_DEBUG_STRING_LEN];
        sprintf(debug_buffer, "[Dispatch Server] Train %d subscribing to [%s]",
                v_p_train_num(train_num), track[subscribed_sensors[i]].name);
        debugprint(debug_buffer, 5);
      }

      subscription_changed = true;

    } else if (req.type == DISPATCHSERVER_SUBSCRIBE_SENSOR_PRINT) {
      sensor_printer = client;
    } else if (req.type == DISPATCHSERVER_SUBSCRIPTION_PRINT) {
      debugprint("[Dispatch Server] Got subscribe printer", 10);
      subscribe_printer = client;
    } else if (req.type == DISPATCHSERVER_STRAIGHTPATHWORKER_INIT) {
      v_train_num train_num = req.data.worker_init.train_num;
      straightpathworkers[train_num] = client;
    } else if (req.type == DISPATCHSERVER_SENSOR_UPDATE) {
      memset((void *)&res, 0, sizeof(dispatchserver_response));
      res.type = DISPATCHSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(dispatchserver_response));

      char *sensor_readings = req.data.sensor_update.sensor_readings;
      unsigned int time = req.data.sensor_update.time;

      int sensor_attribution_backing[MAX_NUM_TRAINS][MAX_SUBSCRIBED_SENSORS];
      circular_buffer sensor_attribution[MAX_NUM_TRAINS];

      for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
        cb_init(&(sensor_attribution[train_num]),
                (void *)sensor_attribution_backing[train_num],
                MAX_SUBSCRIBED_SENSORS, sizeof(int));
      }

      v_train_num sensor_pool[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];

      bool triggering_trains[MAX_NUM_TRAINS];
      memset(triggering_trains, 0, sizeof(bool) * MAX_NUM_TRAINS);

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
                                      (void *)&i, false);
            KASSERT(status != CB_FULL,
                    "Attributed sensors len must be <= MAX_SUBSCRIBED_SENSORS");
            sensor_pool[i] = subscribers[i];

            v_train_num triggering_train = subscribers[i];

            triggering_trains[triggering_train] = true;
          }
        }
      }

      for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
        if (triggering_trains[train_num]) {
          for (int i = 0; i < (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP); i++) {
            if (subscribers[i] == train_num) {
              char debug_buffer[MAX_DEBUG_STRING_LEN];
              sprintf(debug_buffer,
                      "[Dispatch Server] Train %d unsubscribing [%s]",
                      v_p_train_num(train_num), track[i].name);

              debugprint(debug_buffer, 5);
              subscribers[i] = -1;
              subscription_changed = true;
            }
          }
        }

        if (sensor_attribution[train_num].count == 0) {
          continue;
        }

        memset((void *)&res, 0, sizeof(dispatchserver_response));
        res.type = DISPATCHSERVER_GOOD;
        res.data.subscribe_sensor_list.len =
            sensor_attribution[train_num].count;
        res.data.subscribe_sensor_list.time = time;
        cb_to_array(&(sensor_attribution[train_num]),
                    res.data.subscribe_sensor_list.triggered_sensors);
        Reply(straightpathworkers[train_num], (char *)&res,
              sizeof(dispatchserver_response));
      }

      if (sensor_printer != -1) {
        memset((void *)&res, 0, sizeof(dispatchserver_response));
        res.type = DISPATCHSERVER_GOOD;
        res.data.subscribe_sensor_print.time = time;
        memcpy(res.data.subscribe_sensor_print.sensor_pool, sensor_pool,
               sizeof(v_train_num) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
        Reply(sensor_printer, (char *)&res, sizeof(dispatchserver_response));
        sensor_printer = -1;
      }
    }

    if (subscription_changed && subscribe_printer != -1) {
      inform_subscriber(subscribe_printer, subscribers);
      subscription_changed = false;
      subscribe_printer = -1;
    }
  }
}
