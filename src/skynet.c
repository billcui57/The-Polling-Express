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

#include "skynet.h"

void next_sensor(track_node *start, const char *branch_a, const char *branch_b, int *distance,
                 int *next) {
  track_node *cur = start;
  while (true) {
    if (cur->type == NODE_BRANCH) {
      char branch_state = 2;
      if(cur->num < 20) {
        branch_state = branch_a[cur->num];
      } else if (cur->num  >= 153 && cur->num <=156 ){
        branch_state = branch_b[cur->num - 153];
      }
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

int loop[] = {72, 95, 96, 52, 69, 98, 51, 21, 105, 43, 107, 3, 31, 108, 41, 110, 16, 61, 113, 77};

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
      controlserver_request c_req;
      memset(&c_req,0,sizeof(c_req));
      c_req.type = PATHFIND;
      c_req.client.src = req.msg.target.source;
      c_req.client.dest = 77;
      controlserver_response c_res;
      Send(controlserver, (char*)&c_req,sizeof(c_req),(char*)&c_res,sizeof(c_res));
      memset(train.time,0,sizeof(int)*80);
      memset(train.next_time,0,sizeof(int)*80);
      process_path(&train,c_res.client.path,c_res.client.path_len,trainctl);
      train.i=0;
      c_req.client.src = 72;
      c_req.client.dest = req.msg.target.destination;
      Send(controlserver, (char*)&c_req,sizeof(c_req),(char*)&c_res,sizeof(c_res));
      train.dist = (c_res.client.path_dist+req.msg.target.offset)*1000;
      train.out_len = c_res.client.path_len;
      memcpy(train.next_out,c_res.client.path,TRACK_MAX*sizeof(int));
      train.state = 1;
      res.msg.worker.node=train.next[train.i];
      Reply(worker,(char*)&res,sizeof(res));
      Reply(client,(char*)&res,0);
      TrainCommand(trainctl,Time(clock)+5, SPEED, train.train, train.speed);
    } else if (req.type == SKYNET_EVENT) {
      if (req.msg.worker.node == -1) continue;
      train.time[train.i] = req.msg.worker.time;
      if (train.i == train.stop_marker && train.state == 8){
          int time = train.time[train.i]+train.stop_offset/train.vel;
          TrainCommand(trainctl,time, SPEED, train.train, 0);
          train.state = 0;
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
        printf(COM2, "Next Sensor: %s at %d, Stage: %d\r\n", track[train.next[train.i]].name, train.next_time[train.i], train.state);
      }
      restore_cursor();
      if(train.i==train.len){
        if (train.state == 1) {
          process_path(&train, loop, 20, trainctl);
          train.state = 2;
          train.i = 0;
          res.msg.worker.node=train.next[train.i];
          Reply(worker,(char*)&res,sizeof(res));
        } else if (train.state >=2 && train.state <=6) {
          train.state++;
          train.i = 0;
          res.msg.worker.node=train.next[train.i];
          Reply(worker,(char*)&res,sizeof(res));
        } else if (train.state == 7){
          int stopping = 34*train.vel*train.vel+12641*train.vel+115544120;
          stopping/=1000;
          int left = train.dist - stopping + 30000;
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
          save_cursor();
          cursor_to_row(SENSOR_PRED_ROW+1);
          printf(COM2, "Vel: %d, Stop: %d, At: %d + %d",train.vel, stopping, train.next[train.stop_marker], left);
          restore_cursor();
          train.state++;
          train.i = 0;
          res.msg.worker.node=train.next[train.i];
          Reply(worker,(char*)&res,sizeof(res));
        }
      }
    }
  }
}
