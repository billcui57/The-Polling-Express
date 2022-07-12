#pragma once

#include "sensorcourier.h"
#include "syscall.h"
#include "track_node.h"
#include "virtual.h"

typedef enum {
  DISPATCHSERVER_SENSOR_UPDATE,
  DISPATCHSERVER_SUBSCRIBE_SENSOR_LIST,
  DISPATCHSERVER_SUBSCRIBE_SENSOR_PRINT,
  DISPATCHSERVER_SUBSCRIPTION_PRINT,
  DISPATCHSERVER_STRAIGHTPATHWORKER_INIT
} dispatchserver_request_type;

typedef enum {
  NOT_TRIGGERED = -1,
  UNATTRIBUTED = -2
} dispatch_sensor_pool_type;

typedef enum { DISPATCHSERVER_GOOD } dispatchserver_response_type;

#define MAX_SUBSCRIBED_SENSORS 5

typedef struct dispatchserver_response {

  dispatchserver_response_type type;

  union {
    struct {
      int triggered_sensors[MAX_SUBSCRIBED_SENSORS];
      int len;
      unsigned int time;
    } subscribe_sensor_list;

    struct {
      v_train_num sensor_pool[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      unsigned int time;
    } subscribe_sensor_print;

    struct {
      v_train_num subscriptions[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
    } subscription_print;

  } data;

} dispatchserver_response;

typedef struct dispatchserver_request {
  dispatchserver_request_type type;

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

    struct {
      v_train_num train_num;
    } worker_init;

  } data;

} dispatchserver_request;

void dispatchserver();
