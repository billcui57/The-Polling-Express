#pragma once

#include "algorithms.h"
#include "stdbool.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"
#include "trainserver.h"

typedef enum {
  pathserver_GOOD,
  WORKER_PATHFIND
} pathserver_response_type;

typedef enum {
  PATHFIND,
  CONTROL_WORKER,
  CONTROL_WORKER_DONE,
} pathserver_request_type;

typedef enum { TASK_PATHFIND } pathserver_client_task_type;

typedef enum {
  WORKER_PATHFIND_GOOD,
} pathserver_worker_response_type;

typedef struct pathserver_client_task {
  pathserver_client_task_type type;

  union {
    struct {
      task_tid client;
      int src_num;
      int dest_num;
      int offset;
      int min_len;
    } pathfind;
  };
} pathserver_client_task;

typedef struct pathserver_request {

  pathserver_request_type type;

  union {
    struct {
      char src;
      char dest;
      int offset;
      int min_len;
    } client;

    struct {
      pathserver_worker_response_type type;
      task_tid whomfor;
      int path_len;
      int path_dist;
      int path[2 * TRACK_MAX];
    } worker;
  };

} pathserver_request;

typedef struct pathserver_response {
  pathserver_response_type type;

  union {
    struct {
      int src_num;
      int dest_num;
      task_tid whomfor;
      int offset;
      int min_len;
      bool reserved[TRACK_MAX];
    } worker;

    struct {
      int path_len;
      int path_dist;
      int path[2 * TRACK_MAX];
    } client;
  };
} pathserver_response;

void control_server();
