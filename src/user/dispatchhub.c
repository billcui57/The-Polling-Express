#include "dispatchhub.h"

#define MAX_SUBSCRIBERS_PER_SENSOR 10

void dispatchhub() {

  RegisterAs("dispatchhub");

  dispatchhub_request req;
  dispatchhub_response res;
  memset(&res, 0, sizeof(dispatchhub_response));

  task_tid client;

  task_tid all_waiting = -1;

  circular_buffer subscribers[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
  task_tid subscribers_backing[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP]
                              [MAX_SUBSCRIBERS_PER_SENSOR];

  for (unsigned int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
    memset(subscribers_backing[i], 0,
           sizeof(task_tid) * MAX_SUBSCRIBERS_PER_SENSOR);
    cb_init(&(subscribers[i]), (void **)(&(subscribers_backing[i])),
            sizeof(task_tid) * MAX_SUBSCRIBERS_PER_SENSOR);
  }

  for (;;) {
    Receive(&client, (char *)&req, sizeof(dispatchhub_request));

    if (req.type == DISPATCHHUB_SUBSCRIBE_SENSOR_INDIVIDUAL) {

      int subscribe_sensor = req.data.subscribe_individual.subscribed_sensor;

      cb_push_back(&(subscribers[subscribe_sensor]), (void *)client, false);

    } else if (req.type == DISPATCHHUB_SUBSCRIBE_SENSOR_ALL) {

      KASSERT(all_waiting == -1,
              "only one task (the print task) can subscribe to all sensors");

      all_waiting = client;

    } else if (req.type == DISPATCHHUB_SENSOR_UPDATE) {

      int *sensor_readings = req.data.sensor_update.sensor_readings;
      unsigned int time = req.data.sensor_update.time;

      for (int i = 0; i < (NUM_SENSOR_GROUPS * SENSORS_PER_GROUP); i++) {
        int a = i >> 3;
        int b = i & 7;
        if (sensor_readings[a] & 0x80 >> b) {

          while (!cb_is_empty(&(subscribers[i]))) {

            void *subscribers_tid_void;

            cb_pop_front(&(subscribers[i]), &subscribers_tid_void);

            task_tid subscriber_tid = (task_tid)subscribers_tid_void;
            res.data.subscribe_individual.time = time;
            res.type = DISPATCHHUB_GOOD;
            Reply(subscriber_tid, (char *)&res, sizeof(dispatchhub_response));
          }
        }
      }

      if (all_waiting != -1) {
        memset(&res, 0, sizeof(dispatchhub_response));
        res.type = DISPATCHHUB_GOOD;
        res.data.subscribe_all.time = time;
        memcpy(&res.data.subscribe_all.sensor_readings, sensor_readings,
               sizeof(NUM_SENSOR_GROUPS));
        Reply(all_waiting, (char *)&res, sizeof(dispatchhub_response));
      }

      memset(&res, 0, sizeof(dispatchhub_response));
      res.type = DISPATCHHUB_GOOD;
      Reply(client, (char *)&res, sizeof(dispatchhub_response));
    }
  }
}