#pragma once

#include <track_data.h>

typedef struct {
  int time[80];
  int distance[80];
  int next[80];
  int next_time[80];
  int next_out[TRACK_MAX];
  int out_len;
  int i;
  int len;
  int dist;
  int vel;
  int state;
} train_record;


typedef enum {
  SKYNET_TARGET,
  SKYNET_EVENT,
} skynet_msg_type;

typedef struct {
  skynet_msg_type type;
  union {
    struct {
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
