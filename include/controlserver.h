#pragma once

#include "algorithms.h"
#include "stdbool.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"
#include "trainserver.h"

typedef enum { CONTROLSERVER_GOOD } controlserver_response_type;

typedef enum { PATHFIND } controlserver_request_type;
typedef struct controlserver_request {

  controlserver_request_type type;

  char src_name[8];
  char dest_name[8];

} controlserver_request;

typedef struct controlserver_response {
  controlserver_response_type type;
} controlserver_response;

void control_server();
