#include <clockserver.h>
#include <heap.h>
#include <syscall.h>
#include <trainserver.h>
// have a heap of scheduled tasks
// run tasks once time
// run sensors if nothing runable

#define MAX_TRAIN_CMDS 100

train_task *build_task(train_task **free, int time, char a, char b, char len) {
  KASSERT(*free, "No Free Train Task");
  train_task *cur = *free;
  *free = cur->next;
  cur->time = time;
  cur->a = a;
  cur->b = b;
  cur->len = len;
  cur->branch = -1;
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
  bool dirty = false;
  task_tid waiting = -1;
  train_event event;
  memset(&event, 0, sizeof(event));
  for (int i = 0; i < 256; i++)
    event.branches[i] = 2;

  train_msg req;
  train_msg res;
  memset(&res, 0, sizeof(res));
  task_tid client;

  Create(10, task_train_worker);

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
        if (top->branch != -1) {
          event.branches[top->branch] = top->branch_state;
          // dirty = true;
        }
        heap_pop(&h);
        top->next = free;
        free = top;
      } else if (!top || (top && time + 7 < top->time)) {
        res.type = WORKER_SENSOR;
      } else {
        res.type = WORKER;
      }
      Reply(client, (char *)&res, sizeof(res));
    } else if (req.type == WORKER_SENSOR) {
      char *sensor_res = req.data.sensor.sensors;
      for (int i = 0; i < 10; i++) {
        event.sensors[i] |= sensor_res[i] & ~sensors[i];
        dirty |= event.sensors[i];
        sensors[i] = sensor_res[i];
      }
      event.time = req.data.sensor.time;
      Reply(client, (char *)&res, 0);
    } else if (req.type == TRAIN_EVENT) {
      waiting = client;
    } else if (req.type == SPEED) {
      heap_add(&h, build_task(&free, req.data.task.time, req.data.task.data,
                              req.data.task.target, 2));
      Reply(client, (char *)&res, 0);
    } else if (req.type == REVERSE) {
      heap_add(&h, build_task(&free, req.data.task.time, 0,
                              req.data.task.target, 2));
      heap_add(&h, build_task(&free, req.data.task.time + 500, 15,
                              req.data.task.target, 2));
      heap_add(&h, build_task(&free, req.data.task.time + 510,
                              req.data.task.data, req.data.task.target, 2));
      Reply(client, (char *)&res, 0);
    } else if (req.type == SWITCH) {
      train_task *task =
          build_task(&free, req.data.task.time, 33 + req.data.task.data,
                     req.data.task.target, 2);
      task->branch = req.data.task.target;
      task->branch_state = req.data.task.data;
      heap_add(&h, task);
      heap_add(&h, build_task(&free, req.data.task.time + 20, 32, 0, 1));
      Reply(client, (char *)&res, 0);
    } else {
      KASSERT(0, "Unknown train command");
    }

    if (dirty && waiting != -1) {
      Reply(waiting, (char *)&event, sizeof(train_event));
      waiting = -1;
      dirty = false;
      for (int i = 0; i < 10; i++)
        event.sensors[i] = 0;
    }
  }
}

void task_train_worker() {
  task_tid clock = WhoIsBlock("clockserver");
  task_tid uart1 = WhoIsBlock("uart1");
  task_tid parent = MyParentTid();

  train_msg req;
  memset(&req, 0, sizeof(req));
  train_msg res;
  while (true) {
    req.type = WORKER;
    req.data.task.time = Time(clock);
    Send(parent, (char *)&req, sizeof(req), (char *)&res, sizeof(res));
    if (res.type == WORKER_CMD) {
      Putc(uart1, 0, res.data.cmd.a);
      if (res.data.cmd.len == 2)
        Putc(uart1, 0, res.data.cmd.b);
    } else if (res.type == WORKER_SENSOR) {
      req.type = WORKER_SENSOR;
      Putc(uart1, 0, '\x85');
      for (int i = 0; i < 10; i++)
        req.data.sensor.sensors[i] = Getc(uart1, 0);
      req.data.sensor.time = Time(clock);
      Send(parent, (char *)&req, sizeof(req), (char *)&res, 0);
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

void TrainEvent(task_tid tid, train_event *event) {
  train_msg req;
  memset(&req, 0, sizeof(req));
  req.type = TRAIN_EVENT;
  Send(tid, (char *)&req, sizeof(req), (char *)event, sizeof(train_event));
}
