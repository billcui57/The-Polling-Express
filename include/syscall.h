#pragma once

#define SYSCALL_CREATE 1
#define SYSCALL_MYTID 2
#define SYSCALL_MYPARENTTID 3
#define SYSCALL_YIELD 4
#define SYSCALL_EXIT 5
#define SYSCALL_DESTROY 6

typedef struct {
  int priority;
  void (*function)();
} create_args;

int Create(int priority, void (*function)());
int MyTid();
int MyParentTid();
void Yield();
void Exit();
void Destroy();

typedef struct {
  int tid;
  const char *msg;
  int msglen;
  char *reply;
  int rplen;
} send_args;

typedef struct {
  int *tid;
  char *msg;
  int msglen;
} receive_args;

typedef struct {
  int tid;
  const char *reply;
  int rplen;
} reply_args;

int Send(int tid, const char *msg, int msglen, char *reply, int rplen);
int Receive(int *tid, char *msg, int msglen);
int Reply(int tid, const char *reply, int rplen);

int AwaitEvent(int eventid);
