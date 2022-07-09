#pragma once

#include "sensorcourier.h"
#include "syscall.h"
#include "track_node.h"
#include "virtual.h"

typedef enum {
  DISPATCHHUB_SENSOR_UPDATE,
  DISPATCHHUB_SUBSCRIBE_SENSOR_LIST,
  DISPATCHHUB_SUBSCRIBE_SENSOR_PRINT,
  DISPATCHHUB_SKYNET_INIT,
  DISPATCHHUB_SKYNET_TARGET,
} dispatchhub_request_type;

typedef enum {
  NOT_TRIGGERED = -1,
  UNATTRIBUTED = -2
} dispatch_sensor_pool_type;

typedef enum { DISPATCHHUB_GOOD } dispatchhub_response_type;

#define MAX_SUBSCRIBED_SENSORS 5

struct skynet_target {
  char train, speed;
  char source, destination;
  int offset;
};

typedef struct dispatchhub_response {

  dispatchhub_response_type type;

  union {
    struct {
      int triggered_sensors[MAX_SUBSCRIBED_SENSORS];
      int len;
      unsigned int time;
    } subscribe_sensor_list;

    struct {
      v_train_num sensor_pool[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      int sensor_pool_len;
      unsigned int time;
    } subscribe_sensor_print;

    struct skynet_target skynet_target;

  } data;

} dispatchhub_response;

typedef struct dispatchhub_request {
  dispatchhub_request_type type;

  union {

    struct {
      int subscribed_sensors[MAX_SUBSCRIBED_SENSORS];
      int len;
      v_train_num train_num;
    } subscribe_sensor_list;

    struct {
      char sensor_readings[NUM_SENSOR_GROUPS];
      unsigned int time;
    } sensor_update;

    struct skynet_target skynet_target;
  } data;

} dispatchhub_request;

void dispatchhub();
