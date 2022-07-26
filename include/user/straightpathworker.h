#pragma once

#include "debugprinter.h"
#include "layout.h"
#include "neutron.h"

#include <track_data.h>

typedef struct {
  // train info
  char train;
  char speed;
  neutron n;
  // current path state
  int time[160];
  int distance[160];
  int next[160];
  int next_time[160];
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
