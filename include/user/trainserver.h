#pragma once
#include "debugprinter.h"
#include "virtual.h"
#include <syscall.h>

typedef enum {
  WORKER,
  WORKER_CMD,
  WORKER_SENSOR,
  SPEED,
  REVERSE,
  SWITCH,
  SENSOR_EVENT,
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
      unsigned char a, b, len;
      int reqid;
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
  unsigned char a, b, len;
  int branch, branch_state, reqid;
  struct train_task *next;
} train_task;

void task_trainserver();
void task_train_worker();

void TrainCommand(task_tid tid, int time, train_req_def type, int target,
                  int data);
void SensorEvent(task_tid tid, train_event *event);
void BranchEvent(task_tid tid, train_event *event);

void HardReverse(task_tid tid, v_train_num train_num, int time);
