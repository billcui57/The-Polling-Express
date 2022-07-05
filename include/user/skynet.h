#pragma once

#include "layout.h"
#include <track_data.h>

enum train_state {
  TRAIN_STOP,
  TRAIN_TOLOOP,
  TRAIN_SPEEDING,
  TRAIN_PADLOOP,
  TRAIN_FROMLOOP,
};

typedef struct {
  // train info
  char train;
  char speed;
  int vel;
  enum train_state state;
  int state_counter;
  // current path state
  int time[160];
  int distance[160];
  int next[160];
  int next_time[160];
  int i;
  int len;
  int branches[2 * TRACK_MAX];
  int j;
  // destination info
  int next_out[2 * TRACK_MAX];
  int out_len;
  int dist;
  // stopping plan
  int stop_marker;
  int stop_offset;
} train_record;

typedef enum {
  SKYNET_TARGET,
  SKYNET_EVENT,
} skynet_msg_type;

typedef struct {
  skynet_msg_type type;
  union {
    struct {
      char train, speed;
      char source, destination;
      int offset;
    } target;
    struct {
      int node;
      int time;
    } worker;
  } msg;
} skynet_msg;

void task_skynet();
