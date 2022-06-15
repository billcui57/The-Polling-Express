#pragma once
#include <syscall.h>

typedef enum {
  WORKER,
  WORKER_CMD,
  WORKER_SENSOR,
  SENSOR,
  SPEED,
  REVERSE,
  SWITCH,
} train_req_def;

typedef struct {
  train_req_def type;
  union {
    struct {
      int target;
      int data;
      int time;
    } task;
    char sensors[10];
    struct {
      char a, b, len;
    } cmd;
  } data;
} train_msg;

typedef struct train_task {
  int time;
  char a, b, len;
  struct train_task *next;
} train_task;

void task_trainserver();
void task_train_worker();

void TrainCommand(task_tid tid, int time, train_req_def type, int target,
                  int data);
void TrainSensor(task_tid tid, char *sensors);
