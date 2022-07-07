#pragma once

#include "syscall.h"
#include "track_node.h"
#include "virtual.h"

typedef enum {
  DISPATCHHUB_SENSOR_UPDATE,
  DISPATCHHUB_SUBSCRIBE_SENSOR_LIST,
  DISPATCHHUB_SUBSCRIBE_SENSOR_PRINT
} dispatchhub_request_type;

typedef enum { DISPATCHHUB_GOOD } dispatchhub_response_type;

typedef struct dispatchhub_response {

  dispatchhub_response_type type;

  union {
    struct {
      int triggered_sensors[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      int len;
      unsigned int time;
    } subscribe_sensor_list;

    struct {
      int attributed_sensors[MAX_NUM_TRAINS]
                            [NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      int attributed_lens[MAX_NUM_TRAINS];
      int unattributed_sensors[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      int unattributed_len;
      int sensor_pool[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      int sensor_pool_len;
      unsigned int time;
    } subscribe_sensor_print;

  } data;

} dispatchhub_response;

typedef struct dispatchhub_request {
  dispatchhub_request_type type;

  union {

    struct {
      int subscribed_sensors[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      int len;
      v_train_num train_num;
    } subscribe_sensor_list;

    struct {
      int sensor_readings[NUM_SENSOR_GROUPS];
      unsigned int time;
    } sensor_update;

  } data;

} dispatchhub_request;