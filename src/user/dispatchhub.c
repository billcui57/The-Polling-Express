#include "dispatchhub.h"

#define MAX_SUBSCRIBERS 10

void dispatchhub() {

  RegisterAs("dispatchhub");

  dispatchhub_request req;
  dispatchhub_response res;
  memset(&res, 0, sizeof(dispatchhub_response));

  task_tid client;

  task_tid all_waiting = -1;

  // to map client tid to subscriber index
  void *client_tid_subscriber_mapping_backing[MAX_SUBSCRIBERS];
  memset(client_tid_subscriber_mapping_backing, 0,
         sizeof(void *) * MAX_SUBSCRIBERS);

  circular_buffer client_tid_subscriber_mapping;
  cb_init(&client_tid_subscriber_mapping, client_tid_subscriber_mapping_backing,
          MAX_SUBSCRIBERS);

  circular_buffer subscribers[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
  void *subscribers_backing[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP]
                           [MAX_SUBSCRIBERS];

  for (unsigned int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
    memset(subscribers_backing[i], 0, sizeof(void *) * MAX_SUBSCRIBERS);
    cb_init(&(subscribers[i]), (void **)(&(subscribers_backing[i])),
            MAX_SUBSCRIBERS);
  }

  for (;;) {
    Receive(&client, (char *)&req, sizeof(dispatchhub_request));

    if (req.type == DISPATCHHUB_SUBSCRIBE_SENSOR_LIST) {

      int subscriber_index = cb_shallow_linear_search(
          &client_tid_subscriber_mapping, (void *)client);
      if (subscriber_index == -1) {
        cb_push_back(&client_tid_subscriber_mapping, (void *)client, false);
        subscriber_index = cb_shallow_linear_search(
            &client_tid_subscriber_mapping, (void *)client);
        KASSERT(subscriber_index != -1,
                "Should create subscriber client tid mapping correctly");
      }

      int *subscribed_sensors =
          req.data.subscribe_sensor_list.subscribed_sensors;
      int subscribed_sensors_len = req.data.subscribe_sensor_list.len;

      for (unsigned int i = 0; i < subscribed_sensors_len; i++) {
        cb_push_back(&(subscribers[i]), (void *)subscriber_index, false);
      }

    } else if (req.type == DISPATCHHUB_SENSOR_UPDATE) {

      int *sensor_readings = req.data.sensor_update.sensor_readings;
      unsigned int time = req.data.sensor_update.time;

      void *sensor_triggers_backing[client_tid_subscriber_mapping.count]
                                   [NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      circular_buffer sensor_triggers[client_tid_subscriber_mapping.count];

      for (unsigned int i = 0; i < client_tid_subscriber_mapping.count; i++) {
        memset(sensor_triggers_backing[i], 0,
               sizeof(void *) * (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP));
        cb_init(&(sensor_triggers[i]), (void **)(&(sensor_triggers_backing[i])),
                NUM_SENSOR_GROUPS * SENSORS_PER_GROUP);
      }

      for (int i = 0; i < (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP); i++) {
        int a = i >> 3;
        int b = i & 7;
        if (sensor_readings[a] & 0x80 >> b) {

          // TODO: if cb_is_empty already then give to unattributed

          while (!cb_is_empty(&(subscribers[i]))) {

            void *subscribers_id_void;

            cb_pop_front(&(subscribers[i]), &subscribers_id_void);

            task_tid subscriber_id = (task_tid)subscriber_id;

            cb_push_back(&(sensor_triggers[subscriber_id]), (void *)i, false);
          }
        }
      }

      void *client_tids_void[client_tid_subscriber_mapping.count];
      cb_to_array(&client_tid_subscriber_mapping, client_tids_void);

      for (unsigned int i = 0; i < client_tid_subscriber_mapping.count; i++) {

        if (&(sensor_triggers[i]).count == 0) {
          continue;
        }

        res.data.subscribe_sensor_list.len = sensor_triggers[i].count;
        res.data.subscribe_sensor_list.time = time;
        cb_to_array(&(sensor_triggers[i]),
                    res.data.subscribe_sensor_list.triggered_sensors);
        Reply((task_tid)client_tids_void[i], (char *)&res,
              sizeof(dispatchhub_response));

        void *dev_null;
        cb_pop_front(&client_tid_subscriber_mapping, &dev_null);
      }
    }
  }
}