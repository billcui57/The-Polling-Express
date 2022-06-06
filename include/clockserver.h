#pragma once

void clockserver();
void clock_notifier();

typedef enum { TIME, DELAY, DELAYUNTIL, TICK } clockserver_request_type;

typedef struct {
  clockserver_request_type type;
  int data;
} clockserver_request;

typedef struct delay_node {
  int client;
  int time;
  struct delay_node *next;
} delay_node;

int Time(int tid);
int Delay(int tid, int ticks);
int DelayUntil(int tid, int ticks);
