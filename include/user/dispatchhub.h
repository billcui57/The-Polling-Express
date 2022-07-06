#pragma once

#include "syscall.h"
#include "track_node.h"

typedef enum {
  DISPATCHHUB_SENSOR_UPDATE,
  DISPATCHHUB_SUBSCRIBE_SENSOR_INDIVIDUAL,
  DISPATCHHUB_SUBSCRIBE_SENSOR_ALL
} dispatchhub_request_type;

typedef enum { DISPATCHHUB_GOOD } dispatchhub_response_type;

typedef struct dispatchhub_response {

  dispatchhub_response_type type;

  union {
    struct {
      unsigned int time;
    } subscribe_individual;

    struct {
      unsigned int time;
      int sensor_readings[NUM_SENSOR_GROUPS];
    } subscribe_all;
  } data;

} dispatchhub_response;

typedef struct dispatchhub_request {
  dispatchhub_request_type type;

  union {

    struct {
      int subscribed_sensor;
    } subscribe_individual;

    struct {
      int sensor_readings[NUM_SENSOR_GROUPS];
      unsigned int time;
    } sensor_update;

  } data;

} dispatchhub_request;