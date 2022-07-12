#pragma once

#include "nameserver.h"
#include "pathserver.h"
#include "straightpathworker.h"
#include "task.h"
#include "virtual.h"

typedef enum {
  NAVIGATION_REQUEST,
  STRAIGHTPATH_WORKER,
  STRAIGHTPATH_WORKER_DONE,
  PATHFIND_WORKER,
  PATHFIND_WORKER_DONE
} navigationserver_request_type;

typedef enum {
  NAVIGATIONSERVER_GOOD,
  STRAIGHTPATH_WORKER_HERES_WORK,
  PATHFIND_WORKER_HERES_WORK,
  NAVIGATIONSERVER_BUSY,
} navigationserver_response_type;

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
      v_train_num train_num;
      int *path;
      int path_len;
      bool updated_reserved_nodes[TRACK_MAX];
    } pathfindworker_done;

    struct {
      v_train_num train_num;
    } straightpathworker;

    struct {
      v_train_num train_num;
    } straightpathworker_done;

  } data;
} navigationserver_request;

typedef struct navigationserver_response {

  navigationserver_response_type type;
  union {
    struct {
      v_train_num train;
      int speed;
      int *path;
      int path_len;
      int offset;
    } straightpathworker;

    struct {
      v_train_num train;
      char src;
      char dest;
      int offset;
      bool reserved_nodes[TRACK_MAX];
    } pathfindworker;
  } data;

} navigationserver_response;