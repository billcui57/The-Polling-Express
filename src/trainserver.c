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

  int sensors[10] = {0};
  int delta[10] = {0};

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
      char *sensor_res = res.data.sensors;
      for (int i = 0; i < 10; i++) {
        delta[i] |= sensor_res[i] & ~sensors[i];
        sensors[i] = sensor_res[i];
      }
      Reply(client, (char *)&res, 0);
    } else if (req.type == SENSOR) {
      Reply(client, (char *)delta, sizeof(char) * 10);
      for (int i = 0; i < 10; i++)
        delta[i] = 0;
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
      heap_add(&h,
               build_task(&free, req.data.task.time, 33 + req.data.task.data,
                          req.data.task.target, 2));
      heap_add(&h, build_task(&free, req.data.task.time + 20, 32, 0, 1));
      Reply(client, (char *)&res, 0);
    } else {
      KASSERT(0, "Unknown train command");
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
        req.data.sensors[i] = Getc(uart1, 0);
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

void TrainSensor(task_tid tid, char *sensors) {
  train_msg req;
  memset(&req, 0, sizeof(req));
  req.type = SENSOR;
  Send(tid, (char *)&req, sizeof(req), sensors, sizeof(char) * 10);
}
