#pragma once

#include "debugprinter.h"
#include "layout.h"
#include <track_data.h>

enum train_state {
  TRAIN_STOP,
  TRAIN_TOLOOP,
  TRAIN_SPEEDING,
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
  int dist;
  // stopping plan
  int stop_marker;
  int stop_offset;
} train_record;

void task_straightpathworker();
