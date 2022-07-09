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

#include <clockserver.h>
#include <controlserver.h>
#include <kprintf.h>
#include <syscall.h>
#include <trainserver.h>

#include <magic_numbers.h>
#include <track_data.h>

#include "dispatchhub.h"
#include "skynet.h"

void process_path(train_record *t, int *path, int path_len, task_tid trainctl,
                  int time) {
  int dist = 0;
  int j = 0;
  int k = 0;
  for (int i = 0; i < path_len; i++) {
    track_node *cur = &track[path[i]];
    int dir = 0;
    if (cur->type == NODE_SENSOR) {
      t->next[j] = cur->num;
      t->distance[j] = dist * 1000;
      dist = 0;
      j++;
      t->branches[k] = 0;
      k++;
    } else if (cur->type == NODE_BRANCH && i + 1 < path_len) {
      if (cur->edge[DIR_CURVED].dest == &track[path[i + 1]])
        dir = 1;
      t->branches[k] = dir << 8 | cur->num;
      k++;
    } else if (cur->type == NODE_MERGE && i > 0 && false) {
      int rdir = 0;
      if (cur->reverse->edge[DIR_CURVED].dest == track[path[i - 1]].reverse)
        rdir = 1;
      TrainCommand(trainctl, 0, SWITCH, cur->num, rdir);
    }
    dist += cur->edge[dir].dist;
  }
  t->len = j;
  t->branches[k] = -1;
}

void send_branches(train_record *t, task_tid trainctl) {
  if (t->branches[t->j] == -1)
    return;
  t->j++;
  for (; t->branches[t->j] > 0; t->j++) {
    TrainCommand(trainctl, 1, SWITCH, t->branches[t->j] & 0xff,
                 t->branches[t->j] >> 8);
  }
}

void task_skynet() {
  task_tid hub = MyParentTid();
  task_tid clock = WhoIsBlock("clockserver");
  task_tid controlserver = WhoIsBlock("controlserver");
  task_tid trainctl = WhoIsBlock("trainctl");

  dispatchhub_request req;
  dispatchhub_response res;
  memset(&req, 0, sizeof(req));

  train_record train;
  train.vel = 0;
  int next_node = -1;
  while (true) {
    req.type = DISPATCHHUB_SKYNET_INIT;
    Send(hub, (char *)&req, sizeof(req), (char *)&res, sizeof(res));
    train.train = res.data.skynet_target.train;
    train.speed = res.data.skynet_target.speed;
    train.state = TRAIN_TOLOOP;
    train.state_counter = 0;
    controlserver_request c_req;
    memset(&c_req, 0, sizeof(c_req));
    c_req.type = PATHFIND;
    c_req.client.src = res.data.skynet_target.source;
    c_req.client.dest = 74;
    c_req.client.offset = 0;
    c_req.client.min_len = 0;
    controlserver_response c_res;
    Send(controlserver, (char *)&c_req, sizeof(c_req), (char *)&c_res,
         sizeof(c_res));
    memset(train.time, 0, sizeof(int) * 160);
    memset(train.next_time, 0, sizeof(int) * 160);
    process_path(&train, c_res.client.path, c_res.client.path_len, trainctl, 0);
    train.i = 0;
    train.j = 0;
    c_req.client.src = 57;
    c_req.client.dest = res.data.skynet_target.destination;
    c_req.client.offset = res.data.skynet_target.offset;
    c_req.client.min_len = get_stopping(train.train, train.speed) / 1000 + 1;
    Send(controlserver, (char *)&c_req, sizeof(c_req), (char *)&c_res,
         sizeof(c_res));
    memcpy(train.next_out, c_res.client.path, 2 * TRACK_MAX * sizeof(int));
    train.out_len = c_res.client.path_len;
    train.dist = c_res.client.path_dist * 1000;
    train.stop_marker = -1;
    train.stop_offset = 0;
    next_node = train.next[train.i];
    send_branches(&train, trainctl);
    TrainCommand(trainctl, Time(clock) + 5, SPEED, train.train, train.speed);
    while (next_node != -1) {
      req.type = DISPATCHHUB_SUBSCRIBE_SENSOR_LIST;
      req.data.subscribe_sensor_list.subscribed_sensors[0] = next_node;
      req.data.subscribe_sensor_list.len = 1;
      req.data.subscribe_sensor_list.train_num = train.train;
      Send(hub, (char *)&req, sizeof(req), (char *)&res, sizeof(res));
      train.time[train.i] = res.data.subscribe_sensor_list.time;
      if (train.i == train.stop_marker) {
        int time = train.time[train.i] + train.stop_offset / train.vel;
        TrainCommand(trainctl, time, SPEED, train.train, 0);
        train.stop_marker = -1;
        while (train.branches[train.j] != -1)
          send_branches(&train, trainctl);
        next_node = -1;
      } else if (train.i + 1 < train.len) {
        next_node = train.next[train.i + 1];
        send_branches(&train, trainctl);
      }
      if (train.i > 0) {
        int vel = train.distance[train.i] /
                  (train.time[train.i] - train.time[train.i - 1]);
        int pred = train.next_time[train.i] - train.time[train.i];
        cursor_to_pos(TIME_DIFF_ROW, TIME_DIFF_COL, LINE_WIDTH);
        printf(COM2, "Time Diff: %d, Dist Diff: %d, Vel: %d, Svel: %d\r\n",
               pred, vel * pred, vel, train.vel);
        done_print();
        train.vel = (train.vel * 6 + vel * 10) / 16;
      }
      train.i++;
      if (train.i < train.len && train.vel) {
        train.next_time[train.i] =
            train.time[train.i - 1] + (train.distance[train.i] / train.vel);
        cursor_to_pos(SENSOR_PRED_ROW, SENSOR_PRED_COL, LINE_WIDTH);
        printf(COM2, "Next Sensor: %s at %d, Stage: %d (%d)\r\n",
               track[train.next[train.i]].name, train.next_time[train.i],
               train.state, train.state_counter);
        done_print();
      }

      if (train.i == train.len) {
        if (train.state == TRAIN_TOLOOP) {
          process_path(&train, LOOP, LOOP_LEN, trainctl,
                       train.time[train.i - 1]);
          train.state = TRAIN_SPEEDING;
          train.i = 0;
          train.j = 0;
          next_node = train.next[train.i];
          send_branches(&train, trainctl);
        } else if (train.state == TRAIN_SPEEDING && train.state_counter < 1) {
          train.state_counter++;
          train.i = 0;
          train.j = 0;
          next_node = train.next[train.i];
          send_branches(&train, trainctl);
        } else if (train.state == TRAIN_SPEEDING) {
          int left = train.dist - get_stopping(train.train, train.speed);
          if (left > 0) {
            process_path(&train, train.next_out, train.out_len, trainctl,
                         train.time[train.i - 1]);
            for (int i = 0; i < train.len; i++) {
              if (train.distance[i] < left) {
                left -= train.distance[i];
                train.stop_marker = i;
              } else {
                break;
              }
            }
            train.stop_offset = left;
            train.state = TRAIN_FROMLOOP;
            train.i = 0;
            train.j = 0;
            next_node = train.next[train.i];
            send_branches(&train, trainctl);
            cursor_to_pos(VELOCITY_ROW, VELOCITY_COL, LINE_WIDTH);
            printf(COM2, "Vel: %d, Stop: %d, At: %d + %d", train.vel, 0,
                   train.next[train.stop_marker], left);
            done_print();
          } else {
            KASSERT(0, "Not Implemented");
          }
        }
      }
    }
  }
}
