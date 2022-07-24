#include <clockserver.h>
#include <heap.h>
#include <syscall.h>
#include <trainserver.h>
#include <track_data.h>
#include <virtual.h>
// have a heap of scheduled tasks
// run tasks once time
// run sensors if nothing runable

#define MAX_TRAIN_CMDS 100

extern char status[];

int reqid = 0;

train_task *build_task(train_task **free, int time, unsigned char a,
                       unsigned char b, char len) {
  KASSERT(*free, "No Free Train Task");
  train_task *cur = *free;
  *free = cur->next;
  cur->time = time;
  cur->a = a;
  cur->b = b;
  cur->len = len;
  cur->branch = -1;
  cur->reqid = reqid++;
  return cur;
}

void task_trainserver() {
  train_task backing[MAX_TRAIN_CMDS];
  train_task *heap_backing[MAX_TRAIN_CMDS];
  for (int i = 0; i < MAX_TRAIN_CMDS - 1; i++) {
    backing[i].next = &backing[i + 1];
  }
  backing[MAX_TRAIN_CMDS - 1].next = 0;
  train_task *free = &backing[0];
  heap h;
  heap_init(&h, (void **)&heap_backing);

  char sensors[10] = {0};
  bool sensor_dirty = false;
  task_tid sensor_waiting = -1;
  bool branch_dirty = false;
  task_tid branch_waiting = -1;
  train_event event;
  memset(&event, 0, sizeof(event));
  for (int i = 0; i < 20; i++)
    event.branch_a[i] = 2;
  for (int i = 0; i < 4; i++)
    event.branch_b[i] = 2;

  train_msg req;
  train_msg res;
  memset(&res, 0, sizeof(res));
  task_tid client;

  Create(10, "TrainWorker", task_train_worker);

  RegisterAs("trainctl");
  while (true) {
    Receive(&client, (char *)&req, sizeof(req));
    if (req.type == WORKER) {
      train_task *top = heap_peek(&h);
      int time = req.data.task.time;
      if (top && time >= top->time) {
        res.type = WORKER_CMD;
        res.data.cmd.a = top->a;
        res.data.cmd.b = top->b;
        res.data.cmd.len = top->len;
        res.data.cmd.reqid = top->reqid;
        if (top->branch != -1) {
          if (top->branch < 20) {
            event.branch_a[top->branch] = top->branch_state;
          } else if (top->branch >= 153 && top->branch <= 156) {
            event.branch_b[top->branch - 153] = top->branch_state;
          } else {
            KASSERT(0, "Bad Branch");
          }
          branch_dirty = true;
        }
        heap_pop(&h);
        top->next = free;
        free = top;
      } else if ((!top || (top && time + 7 < top->time)) && which_track != '?') {
        res.type = WORKER_SENSOR;
      } else {
        res.type = WORKER;
      }
      Reply(client, (char *)&res, sizeof(res));
    } else if (req.type == WORKER_SENSOR) {
      // debugprint("Got worker sensor");
      char *sensor_res = req.data.sensor.sensors;
      for (int i = 0; i < 10; i++) {
        event.sensors[i] |= sensor_res[i] & ~sensors[i];
        sensor_dirty |= event.sensors[i];
        sensors[i] = sensor_res[i];
      }
      event.time = req.data.sensor.time;
      // debugprint("Replying to worker sensor");
      Reply(client, (char *)&res, 0);
    } else if (req.type == SENSOR_EVENT) {
      sensor_waiting = client;
    } else if (req.type == BRANCH_EVENT) {
      branch_waiting = client;
    } else if (req.type == SPEED) {
      req.data.task.target = v_p_train_num(req.data.task.target);
      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(
          debug_buffer, "[Train Server] train %d, speed %d, time %d, req %d",
          req.data.task.target, req.data.task.data, req.data.task.time, reqid);
      debugprint(debug_buffer, 10);

      heap_add(&h,
               build_task(&free, req.data.task.time, req.data.task.data | 16,
                          req.data.task.target, 2));

      Reply(client, (char *)&res, 0);
    } else if (req.type == REVERSE) {
      req.data.task.target = v_p_train_num(req.data.task.target);
      heap_add(&h, build_task(&free, req.data.task.time, 16,
                              req.data.task.target, 2));
      heap_add(&h, build_task(&free, req.data.task.time + 500, 15,
                              req.data.task.target, 2));
      heap_add(&h,
               build_task(&free, req.data.task.time + 510,
                          req.data.task.data | 16, req.data.task.target, 2));
      Reply(client, (char *)&res, 0);
    } else if (req.type == SWITCH) {
      int t = req.data.task.time;
      int sw = req.data.task.target;
      train_task *task = build_task(&free, t, 33 + req.data.task.data, sw, 2);
      task->branch = sw;
      task->branch_state = req.data.task.data;
      heap_add(&h, task);
      if (req.data.task.target == 153) {
        t += 20;
        train_task *task =
            build_task(&free, t, 33 + !req.data.task.data, 154, 2);
        task->branch = 154;
        task->branch_state = !req.data.task.data;
        heap_add(&h, task);
      } else if (req.data.task.target == 154) {
        t += 20;
        train_task *task =
            build_task(&free, t, 33 + !req.data.task.data, 153, 2);
        task->branch = 153;
        task->branch_state = !req.data.task.data;
        heap_add(&h, task);
      } else if (req.data.task.target == 155) {
        t += 20;
        train_task *task =
            build_task(&free, t, 33 + !req.data.task.data, 156, 2);
        task->branch = 156;
        task->branch_state = !req.data.task.data;
        heap_add(&h, task);
      } else if (req.data.task.target == 156) {
        t += 20;
        train_task *task =
            build_task(&free, t, 33 + !req.data.task.data, 155, 2);
        task->branch = 155;
        task->branch_state = !req.data.task.data;
        heap_add(&h, task);
      }
      t += 20;
      heap_add(&h, build_task(&free, t, 32, 0, 1));
      Reply(client, (char *)&res, 0);
    } else {
      KASSERT(0, "Unknown train command");
    }

    if (sensor_dirty && sensor_waiting != -1) {

      // for (int i = 0; i < 80; i++) {

      //   int a = i >> 3;
      //   int b = i & 7;
      //   if (event.sensors[a] & 0x80 >> b) {

      //     KASSERT(0, "YAY");

      //     // printf(COM2, "Triggered: [%c%d]\r\n", (char)('A' + (i >> 4)),
      //     //        (int)((i & 0xF) + 1));
      //   }
      // }

      Reply(sensor_waiting, (char *)&event, sizeof(train_event));
      sensor_waiting = -1;
      sensor_dirty = false;
      for (int i = 0; i < 10; i++)
        event.sensors[i] = 0;
    }

    if (branch_dirty && branch_waiting != -1) {
      Reply(branch_waiting, (char *)&event, sizeof(train_event));
      branch_waiting = -1;
      branch_dirty = false;
    }
  }
}

// #define DUMMY

void task_train_worker() {
  task_tid clock = WhoIsBlock("clockserver");
  task_tid uart1 = WhoIsBlock("uart1");
  task_tid parent = MyParentTid();

  train_msg req;
  memset(&req, 0, sizeof(req));
  train_msg res;
  int i = 0;
  while (true) {
    req.type = WORKER;
    req.data.task.time = Time(clock);
    // debugprint("Back here");
    Send(parent, (char *)&req, sizeof(req), (char *)&res, sizeof(res));
    // debugprint("Got work");
    if (res.type == WORKER_CMD) {
      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer, "[Train Worker] Sending %d %d %d @ %d for %d",
              res.data.cmd.a, res.data.cmd.b, res.data.cmd.len,
              req.data.task.time, res.data.cmd.reqid);
      debugprint(debug_buffer, 10);
#ifndef DUMMY
      Putc(uart1, 0, res.data.cmd.a);
      if (res.data.cmd.len == 2)
        Putc(uart1, 0, res.data.cmd.b);
#endif
    } else if (res.type == WORKER_SENSOR) {
      // debugprint("Before putc");
      status[0] = 'A' + (i % 26);
      i++;
      req.type = WORKER_SENSOR;
#ifndef DUMMY
      Putc(uart1, 0, '\x85');
      // debugprint("Before polling sensors");
      for (int i = 0; i < 10; i++)
        req.data.sensor.sensors[i] = Getc(uart1, 0);
      // debugprint("After polling sensors");
      status[0] = '+';
      req.data.sensor.time = Time(clock);
      // debugprint("After time");
      status[0] = '-';
#else
      int z = i;
      for (int i = 0; i < 10; i++)
        req.data.sensor.sensors[i] = z % 2 ? 0xFF : 0x00;
      req.data.sensor.time = Delay(clock, 5);
#endif
      Send(parent, (char *)&req, sizeof(req), (char *)&res, 0);
      // debugprint("After send");
    } else if (res.type == WORKER) {
      Delay(clock, 1);
    } else {
      KASSERT(0, "Unknown train worker command");
    }
  }
}

void TrainCommand(task_tid tid, int time, train_req_def type, int target,
                  int data) {
  train_msg req;
  memset(&req, 0, sizeof(req));
  req.type = type;
  req.data.task.time = time;
  req.data.task.target = target;
  req.data.task.data = data;
  int junk = 0;
  Send(tid, (char *)&req, sizeof(req), (char *)&junk, 0);
}

void SensorEvent(task_tid tid, train_event *event) {
  train_msg req;
  memset(&req, 0, sizeof(req));
  req.type = SENSOR_EVENT;
  Send(tid, (char *)&req, sizeof(req), (char *)event, sizeof(train_event));
}

void BranchEvent(task_tid tid, train_event *event) {
  train_msg req;
  memset(&req, 0, sizeof(req));
  req.type = BRANCH_EVENT;
  Send(tid, (char *)&req, sizeof(req), (char *)event, sizeof(train_event));
}

void HardReverse(task_tid tid, v_train_num train_num, int time) {
  TrainCommand(tid, time, SPEED, train_num, 15);
}
