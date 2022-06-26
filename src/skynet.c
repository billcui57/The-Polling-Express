/*
Responsibilities:
Pathfinding:
   reserves in slices until next branch
   release after train passes
   landmark matching
Need a train velocity calculator
   predict future train location / stopping distance
   sensor handler
   update branches after sensors
Trainctl
   keep current state of track
   emit events when track state changes
   timestamp is last sensor trigger
      if missed updates, need to extend distance
   will need to handle teleporting sensors

 */

#include <kprintf.h>
#include <syscall.h>
#include <trainserver.h>

#include <track_data.h>

#include "skynet.h"

void next_sensor(track_node *start, const char *branches, int *distance,
                 int *next) {
  track_node *cur = start;
  while (true) {
    if (cur->type == NODE_BRANCH) {
      char branch_state = branches[cur->num];
      if (branch_state > 1) {
        *distance = 0;
        *next = -1;
        return;
      }
      *distance += cur->edge[branch_state].dist;
      cur = cur->edge[branch_state].dest;
    } else if (cur->type == NODE_EXIT) {
      *distance = 0;
      *next = -1;
      return;
    } else {
      *distance += cur->edge[DIR_AHEAD].dist;
      cur = cur->edge[DIR_AHEAD].dest;
    }
    KASSERT(cur, "No Exit");
    if (cur->type == NODE_SENSOR) {
      *next = cur->num;
      return;
    }
  }
}

void task_skynet() {
  task_tid trainctl = WhoIsBlock("trainctl");
  train_event event;

  train_record train;
  train.next = -1;
  train.distance = 0;

  track_node tracka[TRACK_MAX];
  init_tracka(tracka);

  while (true) {
    bool step = false;
    TrainEvent(trainctl, &event);
    save_cursor();
    printf(COM2, "Event at %d [Next: %d]: ", event.time, train.next);
    for (int i = 0; i < 80; i++) {
      int a = i >> 3;
      int b = i & 7;
      if (event.sensors[a] & 0x80 >> b) {
        printf(COM2, "%c%d ", 'A' + (a >> 1), ((a & 1) << 3) + b + 1);
        if (train.next == -1)
          train.next = i;
        if (train.next == i)
          step = true;
      }
    }
    printf(COM2, "\r\n");
    restore_cursor();
    if (!step)
      continue;
    int next_distance, next_destination, next_time;
    next_distance = 0;
    next_time = 0;
    next_sensor(&tracka[train.next], event.branches, &next_distance,
                &next_destination);
    next_distance *= 1000;
    if (train.distance) {
      int pred_delta = event.time - train.next_time;
      int time_delta = event.time - train.time;
      int vel = train.distance / time_delta;
      // printf(COM2, "Last: %d, Pred: %d, Dist: %d\r\n",train.time,
      // train.next_time, train.distance);
      printf(COM2, "Time Diff: %d, Dist Diff: %d, Vel: %d\r\n", pred_delta,
             vel * pred_delta, vel);
      if (next_distance) {
        next_time =
            event.time + ((next_distance * time_delta) / train.distance);
        printf(COM2, "Next Sensor: %s at %d\r\n", tracka[next_destination].name,
               next_time);
      }
      restore_cursor();
    }
    train.distance = next_distance;
    train.next = next_destination;
    train.next_time = next_time;
    train.time = event.time;
  }
}
