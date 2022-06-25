#pragma once

#include "heap.h"
#include "stdbool.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"

typedef enum { PATHFINDERSERVER_GOOD } pathfinderserver_response_type;

typedef struct pathfinderserver_request {
  int src_num;
  int dest_num;
} pathfinderserver_request;

typedef struct pathfinderserver_response {
  pathfinderserver_response_type type;
  char next_step[5];
} pathfinderserver_response;

void dijkstra(track_node *track, track_node *src, track_node *dest,
              track_node **prev);

void pathfinder_server();
