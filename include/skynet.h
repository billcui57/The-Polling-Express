#pragma once
typedef struct {
  int time;
  int distance;
  int next;
  int next_time;
  int vel;
} train_record;

void task_skynet();
