#pragma once

#include "attributioncourier.h"
#include "clockserver.h"
#include "nameserver.h"
#include "pathworker.h"
#include "straightpathworker.h"
#include "task.h"
#include "trainserver.h"
#include "virtual.h"

typedef enum {
  FOUND_PATH,
  NO_PATH_WITH_RESERVE,
  NO_PATH_AT_ALL
} pathfind_result_t;

typedef enum {
  NAVIGATION_REQUEST,
  STRAIGHTPATH_WORKER,
  STRAIGHTPATH_WORKER_DONE,
  PATHFIND_WORKER,
  PATHFIND_WORKER_DONE,
  NAVIGATIONSERVER_ATTRIBUTION_COURIER,
  WHOAMI,
  GET_RESERVATIONS,
  GET_TRAIN_STATE,
  REGISTER_LOCATION
} navigationserver_request_type;

typedef enum {
  NAVIGATIONSERVER_GOOD,
  NAVIGATIONSERVER_NO_PATH,
  NAVIGATIONSERVER_NEED_REGISTER,
  STRAIGHTPATH_WORKER_HERES_WORK,
  PATHFIND_WORKER_HERES_WORK,
  NAVIGATIONSERVER_BUSY,
} navigationserver_response_type;

typedef struct train_state_t {
  v_train_num train_num;
  int reged_at_num;
  int dest_num;
  bool reversed;
  int reged_at_offset;
  int dest_offset;
} train_state_t;

typedef enum { PATHFIND, STRAIGHTPATH } navigationserver_worker_type;
typedef struct navigationserver_request {

  navigationserver_request_type type;

  union {
    struct {
      v_train_num train;
      int destination_num;
      int offset;
    } navigation_request;

    struct {
      v_train_num train_num;
    } pathfindworker;

    struct {
      pathfind_result_t pathfind_result;
      v_train_num train_num;
      int path[TRACK_MAX];
      int path_len;
    } pathfindworker_done;

    struct {
      v_train_num train_num;
    } straightpathworker;

    struct {
      v_train_num train_num;
    } straightpathworker_done;

    struct {
      navigationserver_worker_type worker_type;
    } whoami;

    struct {
      v_train_num train_num;
      int node_num;
    } register_location;

    struct {
      v_train_num sensor_pool[NUM_SENSOR_GROUPS * SENSORS_PER_GROUP];
      unsigned int time;
    } attribution_courier;

  } data;
} navigationserver_request;

typedef struct navigationserver_response {

  navigationserver_response_type type;
  union {
    struct {
      int path[TRACK_MAX];
      int path_len;
      int path_dist;
      int delay_time;
    } straightpathworker;

    struct {
      v_train_num train;
      int src_num;
      int dest_num;
      int offset;
      bool reserved_nodes[TRACK_MAX];
      int delay_time;
    } pathfindworker;

    struct {
      v_train_num reservations[TRACK_MAX];
    } get_reservations;

    struct {
      v_train_num train;
    } whoami;

    struct {
      train_state_t train_states[MAX_NUM_TRAINS];
    } get_train_state;
  } data;

} navigationserver_response;

void navigation_server();