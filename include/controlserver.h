#pragma once

#include "algorithms.h"
#include "stdbool.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"
#include "trainserver.h"

typedef enum {
  CONTROLSERVER_GOOD,
  CONTROLSERVER_NO_PATH,
  WORKER_PATHFIND
} controlserver_response_type;

typedef enum {
  PATHFIND,
  CONTROL_WORKER,
  CONTROL_WORKER_DONE,
} controlserver_request_type;

typedef enum { TASK_PATHFIND } controlserver_client_task_type;

typedef enum {
  WORKER_PATHFIND_GOOD,
  WORKER_PATHFIND_NO_PATH
} controlserver_worker_response_type;

typedef struct controlserver_client_task {
  controlserver_client_task_type type;

  union {
    struct {
      task_tid client;
      int src_num;
      int dest_num;
      int offset;
    } pathfind;
  };
} controlserver_client_task;

typedef struct controlserver_request {

  controlserver_request_type type;

  union {
    struct {
      char src;
      char dest;
      int offset;
    } client;

    struct {
      controlserver_worker_response_type type;
      task_tid whomfor;
      int path_len;
      int path_dist;
      int path[TRACK_MAX];
    } worker;
  };

} controlserver_request;

typedef struct controlserver_response {
  controlserver_response_type type;

  union {
    struct {
      int src_num;
      int dest_num;
      task_tid whomfor;
      int offset;
      bool reserved[TRACK_MAX];
    } worker;

    struct {
      int path_len;
      int path_dist;
      int path[TRACK_MAX];
    } client;
  };
} controlserver_response;

void control_server();
