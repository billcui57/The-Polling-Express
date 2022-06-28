#pragma once
#include <syscall.h>

typedef enum {
  WORKER,
  WORKER_CMD,
  WORKER_SENSOR,
  SPEED,
  REVERSE,
  SWITCH,
  TRAIN_EVENT,
  BRANCH_EVENT,
} train_req_def;

typedef struct {
  train_req_def type;
  union {
    struct {
      int target;
      int data;
      int time;
    } task;
    struct {
      int time;
      char sensors[10];
    } sensor;
    struct {
      char a, b, len;
    } cmd;
  } data;
} train_msg;

typedef struct {
  int time;
  char sensors[10];
  char branch_a[20];
  char branch_b[4];
} train_event;

typedef struct train_task {
  int time;
  char a, b, len;
  int branch, branch_state;
  struct train_task *next;
} train_task;

void task_trainserver();
void task_train_worker();

void TrainCommand(task_tid tid, int time, train_req_def type, int target,
                  int data);
void TrainEvent(task_tid tid, train_event *event);
