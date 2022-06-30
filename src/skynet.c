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
#include <controlserver.h>
#include <clockserver.h>

#include <track_data.h>
#include <magic_numbers.h>

#include "skynet.h"

void task_skynet_worker() {
  task_tid parent = MyParentTid();
  task_tid trainctl = WhoIsBlock("trainctl");
  skynet_msg req;
  memset(&req,0,sizeof(req));
  req.type = SKYNET_EVENT;
  req.msg.worker.node = -1;
  skynet_msg res;
  train_event event;
  int watching = -1;
  while (true) {
    Send(parent, (char*)&req, sizeof(skynet_msg), (char*)&res, sizeof(skynet_msg));
    watching = res.msg.worker.node;
    bool found = false;
    while(!found) {
      TrainEvent(trainctl, &event);
      save_cursor();
      cursor_to_row(EVENT_ANNOUNCE_ROW);
      printf(COM2, "Event at %d [Next: %d]: ", event.time, watching);
      for (int i = 0; i < 80; i++) {
        int a = i >> 3;
        int b = i & 7;
        if (event.sensors[a] & 0x80 >> b) {
          printf(COM2, "%c%d ", 'A' + (i >> 4), (i & 15) + 1);
          if (watching == -1)
            watching = i;
          if (watching == i) {
            found = true;
            req.msg.worker.node = watching;
            req.msg.worker.time = event.time;
          }
        }
      }
      restore_cursor();
    }
  }
}

void process_path(train_record *t, int *path, int path_len, task_tid trainctl){
  int dist = 0;
  int j = 0;
  for(int i=0;i<path_len;i++) {
    track_node *cur = &track[path[i]];
    int dir = 0;
    if(cur->type == NODE_SENSOR){
      t->next[j]=cur->num;
      t->distance[j]=dist*1000;
      dist=0;
      j++;
    } else if (cur->type == NODE_BRANCH && i+1<path_len) {
      if (cur->edge[DIR_CURVED].dest == &track[path[i+1]]) dir =1;
      TrainCommand(trainctl,0, SWITCH, cur->num, dir);
    } else if (cur->type == NODE_MERGE && i>0 && false){
      int rdir = 0;
      if (cur->reverse->edge[DIR_CURVED].dest == track[path[i-1]].reverse)rdir = 1;
      TrainCommand(trainctl,0, SWITCH, cur->num, rdir);
    }
    dist += cur->edge[dir].dist;
  }
  t->len = j;
}

void task_skynet() {
  task_tid worker = Create(10, task_skynet_worker);

  task_tid clock = WhoIsBlock("clockserver");
  task_tid controlserver = WhoIsBlock("controlserver");
  task_tid trainctl = WhoIsBlock("trainctl");


  skynet_msg req;
  skynet_msg res;
  memset(&res,0,sizeof(res));
  task_tid client;

  train_record train;
  train.vel = 0;
  RegisterAs("skynet");
  while (true) {
    Receive(&client, (char*)&req, sizeof(skynet_msg));
    if (req.type == SKYNET_TARGET) {
      train.train = req.msg.target.train;
      train.speed = req.msg.target.speed;
      train.state = TRAIN_TOLOOP;
      train.state_counter = 0;
      controlserver_request c_req;
      memset(&c_req,0,sizeof(c_req));
      c_req.type = PATHFIND;
      c_req.client.src = req.msg.target.source;
      c_req.client.dest = 77;
      c_req.client.offset = 0;
      controlserver_response c_res;
      Send(controlserver, (char*)&c_req,sizeof(c_req),(char*)&c_res,sizeof(c_res));
      memset(train.time,0,sizeof(int)*80);
      memset(train.next_time,0,sizeof(int)*80);
      process_path(&train,c_res.client.path,c_res.client.path_len,trainctl);
      train.i=0;
      c_req.client.src = 72;
      c_req.client.dest = req.msg.target.destination;
      c_req.client.offset = req.msg.target.offset;
      Send(controlserver, (char*)&c_req,sizeof(c_req),(char*)&c_res,sizeof(c_res));
      memcpy(train.next_out,c_res.client.path,TRACK_MAX*sizeof(int));
      train.out_len = c_res.client.path_len;
      train.dist = c_res.client.path_dist*1000;
      train.stop_marker = -1;
      train.stop_offset = 0;
      res.msg.worker.node=train.next[train.i];
      Reply(worker,(char*)&res,sizeof(res));
      Reply(client,(char*)&res,0);
      TrainCommand(trainctl,Time(clock)+5, SPEED, train.train, train.speed);
    } else if (req.type == SKYNET_EVENT) {
      if (req.msg.worker.node == -1) continue;
      train.time[train.i] = req.msg.worker.time;
      if (train.i == train.stop_marker){
          int time = train.time[train.i]+train.stop_offset/train.vel;
          TrainCommand(trainctl,time, SPEED, train.train, 0);
          train.stop_marker = -1;
      } else if (train.i+1<train.len){
        res.msg.worker.node = train.next[train.i+1];
        Reply(worker,(char*)&res,sizeof(res));
      }
      save_cursor();
      if (train.i>0){
        int vel = train.distance[train.i]/(train.time[train.i]-train.time[train.i-1]);
        int pred = train.next_time[train.i]-train.time[train.i];
        cursor_to_row(TIME_DIFF_ROW);
        printf(COM2, "Time Diff: %d, Dist Diff: %d, Vel: %d, Svel: %d\r\n", pred,
            vel * pred, vel, train.vel);
        train.vel = (train.vel * 6 + vel * 10) / 16;
      }
      train.i++;
      if (train.i<train.len && train.vel){
        train.next_time[train.i] = train.time[train.i-1]+(train.distance[train.i]/train.vel);
        cursor_to_row(SENSOR_PRED_ROW);
        printf(COM2, "Next Sensor: %s at %d, Stage: %d (%d)\r\n", track[train.next[train.i]].name, train.next_time[train.i], train.state, train.state_counter);
      }
      restore_cursor();
      if(train.i==train.len){
        if (train.state == TRAIN_TOLOOP) {
          process_path(&train, LOOP, LOOP_LEN, trainctl);
          train.state = TRAIN_SPEEDING;
          train.i = 0;
          res.msg.worker.node=train.next[train.i];
          Reply(worker,(char*)&res,sizeof(res));
        } else if (train.state == TRAIN_SPEEDING && train.state_counter < 3) {
          train.state_counter++;
          train.i = 0;
          res.msg.worker.node=train.next[train.i];
          Reply(worker,(char*)&res,sizeof(res));
        } else if (train.state == TRAIN_SPEEDING){
          int left = train.dist - get_stopping(train.train, train.speed);
          if (left > 0){
            process_path(&train,train.next_out,train.out_len,trainctl);
            for(int i=0;i<train.len;i++){
              if(train.distance[i] < left){
                left -= train.distance[i];
                train.stop_marker = i;
              } else {
                break;
              }
            }
            train.stop_offset = left;
            train.state = TRAIN_FROMLOOP;
            train.i = 0;
            res.msg.worker.node=train.next[train.i];
            Reply(worker,(char*)&res,sizeof(res));
            save_cursor();
            cursor_to_row(SENSOR_PRED_ROW+1);
            printf(COM2, "Vel: %d, Stop: %d, At: %d + %d",train.vel, 0, train.next[train.stop_marker], left);
            restore_cursor();
          } else {
            KASSERT(0, "Not Implemented");
          }
        }
      }
    }
  }
}
