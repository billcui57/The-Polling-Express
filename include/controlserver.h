#pragma once

#include "heap.h"
#include "stdbool.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"
#include "trainserver.h"

typedef enum {
  CONTROLSERVER_GOOD,
  WORKER_PATHFIND
} controlserver_response_type;
typedef enum { CONTROLSERVER_WORKER, CLIENT } controlserver_request_type;
typedef struct controlserver_request {

  controlserver_request_type type;
  union {
    struct {
      char src_name[8];
      char dest_name[8];
    } cmd;
    struct {
      unsigned int time;
    } worker;

  } data;

} controlserver_request;

typedef struct controlserver_response {
  controlserver_response_type type;

  union {
    struct {
      int next_step_num;
    } cmd;
    struct {
      int src_num;
      int dest_num;
      int *occupied;
    } worker_pathfind;
  } data;

} controlserver_response;

void dijkstra(track_node *track, track_node *src, track_node *dest,
              track_node **prev);

void pathfinder_server();
