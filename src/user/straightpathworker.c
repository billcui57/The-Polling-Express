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
#include "neutron.h"
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
    } else if (cur->type == NODE_MERGE && i > 0) {
      int rdir = 0;
      if (cur->reverse->edge[DIR_CURVED].dest == track[path[i - 1]].reverse)
        rdir = 1;
      t->branches[k] = rdir << 8 | cur->num;
      k++;
    }
    dist += cur->edge[dir].dist;
  }
  t->len = j;
  t->branches[k] = -1;
}

void send_branches(train_record *t, int time, task_tid trainctl) {
  debugprint("Send branches", STRAIGHT_PATH_WORKER_DEBUG);
  if (t->j >= 0 && t->branches[t->j] == -1)
    return;
  t->j++;
  for (; t->branches[t->j] > 0; t->j++) {
    TrainCommand(trainctl, time, SWITCH, t->branches[t->j] & 0xff,
                 t->branches[t->j] >> 8);
  }
}

void task_straightpathworker() {
  task_tid navigationserver = MyParentTid();
  task_tid dispatchserver = WhoIsBlock("dispatchserver");
  task_tid clock = WhoIsBlock("clockserver");
  task_tid trainctl = WhoIsBlock("trainctl");

  train_record train;

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
    nav_req.type = STRAIGHTPATH_WORKER;
    nav_req.data.straightpathworker.train_num = train.train;
    Send(navigationserver, (char *)&nav_req, sizeof(navigationserver_request),
         (char *)&nav_res, sizeof(navigationserver_response));

    int path_dist = nav_res.data.straightpathworker.path_dist;
    KASSERT(path_dist > 0, "Negative Distance");
    int path[TRACK_MAX];
    memcpy(path, nav_res.data.straightpathworker.path, sizeof(int) * TRACK_MAX);
    int path_len = nav_res.data.straightpathworker.path_len;
    int delay_time = nav_res.data.straightpathworker.delay_time;

    char debug_buffer[MAX_DEBUG_STRING_LEN];

    sprintf(debug_buffer, "Train %d Delaying for %d ticks before start",
            train.train, delay_time);
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);
    Delay(clock, delay_time);

    sprintf(debug_buffer, "Got work for train %d", v_p_train_num(train.train));
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);
    memset(debug_buffer, 0, sizeof(char) * MAX_DEBUG_STRING_LEN);
    sprintf(debug_buffer,
            "Train %d Path: [%s]->[%s] Path Dist: %d Path Len : %d",
            train.train, track[path[0]].name, track[path[path_len - 1]].name,
            path_dist, path_len);
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);

    memset(train.time, 0, sizeof(int) * 160);
    memset(train.next_time, 0, sizeof(int) * 160);
    process_path(&train, path, path_len, trainctl, 0);
    train.j = -1;
    train.dist = path_dist * 1000;
    create_neutron(&train.n, train.train, train.dist, false);
    train.speed = train.n.speed;
    int dist = 0;
    train.stop_marker = -1;
    for (int i = 0; i < train.len; i++) {
      if (dist + train.distance[i] < train.n.dist_a + train.n.dist_b) {
        dist += train.distance[i];
        train.stop_marker = i;
      } else {
        break;
      }
    }
    train.stop_offset =
        train.n.time_a + train.n.time_b - find_time(&train.n, dist);
    sprintf(debug_buffer, "Train %d Neutron: %d(%d) %d(%d) %d | %d + %d",
            train.train, train.n.time_a, train.n.dist_a, train.n.time_b,
            train.n.dist_b, train.n.time_c, train.stop_marker,
            train.stop_offset);
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);

    send_branches(&train, Time(clock), trainctl); // to first sensor
    send_branches(&train, Time(clock), trainctl); // 1 sensor margin

    sprintf(debug_buffer, "Train %d Send train speed", train.train);
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);
    int start_time = Time(clock);
    TrainCommand(trainctl, start_time, SPEED, train.train, train.speed);
    int marker_time = start_time;
    if (!train.n.dist_b && train.len) {
      // short stop with atleast one sensor
      dis_req.type = DISPATCHSERVER_SUBSCRIBE_SENSOR_LIST;
      dis_req.data.subscribe_sensor_list.subscribed_sensors[0] = train.next[0];
      dis_req.data.subscribe_sensor_list.len = 1;
      dis_req.data.subscribe_sensor_list.train_num = train.train;
      Send(dispatchserver, (char *)&dis_req, sizeof(dis_req),
               (char *)&dis_res, sizeof(dis_res));
      int lag = dis_res.data.subscribe_sensor_list.time - 100 - start_time;
      adjust_offset(&train.n, train.train, lag);
      marker_time = start_time;
      train.stop_offset = train.n.time_a + train.n.time_b;
      sprintf(debug_buffer, "Neutron Rebuild: %d(%d) %d(%d) %d", train.n.time_a,
            train.n.dist_a, train.n.time_b, train.n.dist_b, train.n.time_c);
      debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);
    } else if (train.stop_marker != -1) {
      bool skip = false;
      for (int i = 0; i <= train.stop_marker; i++) {
        if (!skip) {
          dis_req.type = DISPATCHSERVER_SUBSCRIBE_SENSOR_LIST;
          dis_req.data.subscribe_sensor_list.subscribed_sensors[0] =
              train.next[i];
          dis_req.data.subscribe_sensor_list.len = 1;
          if (i + 1 < train.len) {
            dis_req.data.subscribe_sensor_list.subscribed_sensors[1] =
                train.next[i + 1];
            dis_req.data.subscribe_sensor_list.len = 2;
          }
          dis_req.data.subscribe_sensor_list.train_num = train.train;
          Send(dispatchserver, (char *)&dis_req, sizeof(dis_req),
               (char *)&dis_res, sizeof(dis_res));
        } else {
          skip = false;
        }
        train.time[i] = dis_res.data.subscribe_sensor_list.time;
        if (i == 0) train.time[i]-= 100;
        if (i + 1 < train.len &&
            dis_res.data.subscribe_sensor_list.triggered_sensors[0] ==
                train.next[i + 1]) {
          skip = true;
        }
        send_branches(&train, Time(clock), trainctl);
      }
      marker_time = train.time[train.stop_marker];
    }

    int time = marker_time + train.stop_offset;
    int stop_time = train.n.time_c;

    sprintf(debug_buffer, "Train %d Stopping train", train.train);
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);

    TrainCommand(trainctl, time, SPEED, train.train, 0);
    train.stop_marker = -1;
    while (train.branches[train.j] != -1)
      send_branches(&train, Time(clock), trainctl);
    DelayUntil(clock, time + stop_time);
    nav_req.type = STRAIGHTPATH_WORKER_DONE;
    nav_req.data.straightpathworker_done.train_num = train.train;

    sprintf(debug_buffer, "Train %d Done", train.train);
    debugprint(debug_buffer, STRAIGHT_PATH_WORKER_DEBUG);

    Send(navigationserver, (char *)&nav_req, sizeof(navigationserver_request),
         (char *)&nav_res, sizeof(navigationserver_response));
  }
}
