#pragma once

#include "heap.h"
#include "stdbool.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"

typedef enum { PATHFINDERSERVER_GOOD } pathfinderserver_response_type;

typedef struct pathfinderserver_request {
  char src_name[5];
  char dest_name[5];
} pathfinderserver_request;

typedef struct pathfinderserver_response {
  pathfinderserver_response_type type;
  int next_step_num;
} pathfinderserver_response;

void dijkstra(track_node *track, track_node *src, track_node *dest,
              track_node **prev);

void pathfinder_server();
