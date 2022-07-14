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
#include <kprintf.h>
#include <pathworker.h>
#include <syscall.h>
#include <trainserver.h>

#include <magic_numbers.h>
#include <track_data.h>

#include "dispatchserver.h"
#include "straightpathworker.h"

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
  debugprint("[Straight Path] Send branches");
  if (t->branches[t->j] == -1)
    return;
  t->j++;
  for (; t->branches[t->j] > 0; t->j++) {
    TrainCommand(trainctl, 1, SWITCH, t->branches[t->j] & 0xff,
                 t->branches[t->j] >> 8);
  }
}

int get_stopping_time() { return 200; }

void task_straightpathworker() {
  task_tid navigationserver = MyParentTid();
  task_tid dispatchserver = WhoIsBlock("dispatchserver");
  task_tid clock = WhoIsBlock("clockserver");
  task_tid trainctl = WhoIsBlock("trainctl");

  train_record train;
  train.vel = 3000;
  int next_node = -1;

  dispatchserver_request dis_req;
  dispatchserver_response dis_res;
  memset(&dis_req, 0, sizeof(dispatchserver_request));

  navigationserver_request nav_req;
  navigationserver_response nav_res;
  memset(&nav_req, 0, sizeof(navigationserver_request));

  nav_req.type = WHOAMI;
  nav_req.data.whoami.worker_type = STRAIGHTPATH;
  Send(navigationserver, (char *)&nav_req, sizeof(navigationserver_request),
       (char *)&nav_res, sizeof(navigationserver_response));

  train.train = nav_res.data.whoami.train;

  while (true) {

    navigationserver_request nav_req;
    navigationserver_response nav_res;
    memset(&nav_req, 0, sizeof(navigationserver_request));

    nav_req.type = STRAIGHTPATH_WORKER;
    nav_req.data.straightpathworker.train_num = train.train;
    Send(navigationserver, (char *)&nav_req, sizeof(navigationserver_request),
         (char *)&nav_res, sizeof(navigationserver_response));

    int path_dist = nav_res.data.straightpathworker.path_dist;
    int *path = nav_res.data.straightpathworker.path;
    int path_len = nav_res.data.straightpathworker.path_len;
    int speed = nav_res.data.straightpathworker.speed;

    char debug_buffer[MAX_DEBUG_STRING_LEN];
    sprintf(debug_buffer, "[Straightpathworker] Got work for train %d",
            train.train);
    debugprint(debug_buffer);
    memset(debug_buffer, 0, sizeof(char) * MAX_DEBUG_STRING_LEN);
    sprintf(debug_buffer, "Path Dist: %d Path Len : %d Speed: %d", path_dist,
            path_len, speed);
    debugprint(debug_buffer);

    train.speed = speed;
    memset(train.time, 0, sizeof(int) * 160);
    memset(train.next_time, 0, sizeof(int) * 160);
    process_path(&train, path, path_len, trainctl, 0);
    train.i = 0;
    train.j = -1;
    train.dist = path_dist * 1000;
    int left = train.dist; // - get_stopping(train.train, train.speed);
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
    next_node = train.next[train.i];
    send_branches(&train, trainctl);
    debugprint("[Straight Path] Send train speed");
    TrainCommand(trainctl, Time(clock) + 5, SPEED, train.train, train.speed);
    while (next_node != -1) {
      dis_req.type = DISPATCHSERVER_SUBSCRIBE_SENSOR_LIST;
      dis_req.data.subscribe_sensor_list.subscribed_sensors[0] = next_node;
      dis_req.data.subscribe_sensor_list.len = 1;
      dis_req.data.subscribe_sensor_list.train_num = train.train;
      Send(dispatchserver, (char *)&dis_req, sizeof(dis_req), (char *)&dis_res,
           sizeof(dis_res));
      train.time[train.i] = dis_res.data.subscribe_sensor_list.time;
      if (train.i == train.stop_marker) {
        int time = train.time[train.i] + train.stop_offset / train.vel;
        int stop_time = get_stopping_time();
        debugprint("[Straight Path] Stopping train");
        TrainCommand(trainctl, time, SPEED, train.train, 0);
        train.stop_marker = -1;
        while (train.branches[train.j] != -1)
          send_branches(&train, trainctl);
        next_node = -1;
        DelayUntil(clock, time + stop_time);
      } else if (train.i + 1 < train.len) {
        next_node = train.next[train.i + 1];
        send_branches(&train, trainctl);
      }
      if (train.i > 0) {
        int vel = train.distance[train.i] /
                  (train.time[train.i] - train.time[train.i - 1]);
        train.vel = (train.vel * 6 + vel * 10) / 16;
      }
      train.i++;
      if (train.i < train.len && train.vel) {
        train.next_time[train.i] =
            train.time[train.i - 1] + (train.distance[train.i] / train.vel);
      }
    }
    nav_req.type = STRAIGHTPATH_WORKER_DONE;
    nav_req.data.straightpathworker_done.train_num = train.train;
    debugprint("[Straight Path] Done");
    Send(navigationserver, (char *)&nav_req, sizeof(navigationserver_request),
         (char *)&nav_res, sizeof(navigationserver_response));
  }
}
