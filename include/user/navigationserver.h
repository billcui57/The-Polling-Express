#pragma once

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
  WHOAMI,
  GET_RESERVATIONS
} navigationserver_request_type;

typedef enum {
  NAVIGATIONSERVER_GOOD,
  NAVIGATIONSERVER_NO_PATH,
  STRAIGHTPATH_WORKER_HERES_WORK,
  PATHFIND_WORKER_HERES_WORK,
  NAVIGATIONSERVER_BUSY,
} navigationserver_response_type;

typedef enum { PATHFIND, STRAIGHTPATH } navigationserver_worker_type;
typedef struct navigationserver_request {

  navigationserver_request_type type;

  union {
    struct {
      v_train_num train;
      int speed;
      int source_num;
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

  } data;
} navigationserver_request;

typedef struct navigationserver_response {

  navigationserver_response_type type;
  union {
    struct {
      int speed;
      int path[TRACK_MAX];
      int path_len;
      int path_dist;
    } straightpathworker;

    struct {
      v_train_num train;
      char src;
      char dest;
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
  } data;

} navigationserver_response;

void navigation_server();