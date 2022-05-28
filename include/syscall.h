#pragma once

<<<<<<< HEAD
#include "my_assert.h"
=======
>>>>>>> 22eb0ee (checkpoint - before checking for valid nameserver tid)
#include "my_string.h"
#include "nameserver.h"

#define SYSCALL_IRQ 0
#define SYSCALL_CREATE 1
#define SYSCALL_MYTID 2
#define SYSCALL_MYPARENTTID 3
#define SYSCALL_YIELD 4
#define SYSCALL_EXIT 5
#define SYSCALL_DESTROY 6

#define SYSCALL_SEND 7
#define SYSCALL_RECEIVE 8
#define SYSCALL_REPLY 9

#define EINVALIDPRIORITY -1   // invalid priority
#define ENOTASKDESCRIPTORS -2 // kernel is out of task descriptors

#define EINVALIDTID -1   // tid isn't an active task
#define ENOTREPLYWAIT -2 // target isn't waiting for reply

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

/*
Returns:
-1 if nameserver tid does not exist
0 if good
*/
int RegisterAs(const char *name);

/*
Returns:
-1 if nameserver tid does not exist
tid if good
<<<<<<< HEAD
-2 name does not exist
-3 other errors
=======
1 if task with name does not exist
2 other errors
>>>>>>> 22eb0ee (checkpoint - before checking for valid nameserver tid)
*/
int WhoIs(const char *name);

int AwaitEvent(int eventid);
