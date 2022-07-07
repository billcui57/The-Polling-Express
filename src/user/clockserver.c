#include <clockserver.h>

#include <nameserver.h>

#include <my_event.h>
#include <syscall.h>

void clock_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  clockserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = TICK;
  req.data = 0;
  while (true) {
    AwaitEvent(TIMER_TICK);
    Send(parent, (char *)&req, sizeof(clockserver_request), (char *)&junk, 0);
  }
}

void clockserver() {
  Create(20, clock_notifier);
  int ticks = 0;
  int client;
  clockserver_request req;

  delay_node backing[50];
  for (int i = 0; i < 50 - 1; i++) {
    backing[i].next = &backing[i + 1];
  }
  backing[50 - 1].next = NULL;

  delay_node *free = &backing[0];
  delay_node *head = NULL;

  RegisterAs("clockserver");

  while (true) {
    Receive(&client, (char *)&req, sizeof(clockserver_request));
    if (req.type == TIME) {
      Reply(client, (char *)&ticks, sizeof(int));
    } else if (req.type == DELAY || req.type == DELAYUNTIL) {
      if (req.type == DELAY) {
        if (req.data < 0) {
          int error = -2;
          Reply(client, (char *)&error, sizeof(int));
          continue;
        }
        req.type = DELAYUNTIL;
        req.data = ticks + req.data;
      }
      if (req.data <= ticks) {
        Reply(client, (char *)&ticks, sizeof(int));
        continue;
      }
      KASSERT(free != NULL, "No more delay slots");
      delay_node *cur = free;
      free = free->next;
      cur->client = client;
      cur->time = req.data;
      delay_node **prev = &head;
      while (*prev && (*prev)->time < cur->time)
        prev = &(*prev)->next;
      cur->next = *prev;
      *prev = cur;
    } else if (req.type == TICK) {
      Reply(client, (char *)&ticks, 0);
      ticks++;
      while (head && head->time <= ticks) {
        delay_node *cur = head;
        head = head->next;
        Reply(cur->client, (char *)&ticks, sizeof(int));
        cur->next = free;
        free = cur;
      }
    }
  }
}

int Time(int tid) {
  int ret;
  clockserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = TIME;
  Send(tid, (char *)&req, sizeof(clockserver_request), (char *)&ret,
       sizeof(int));
  return ret;
}

int Delay(int tid, int ticks) {
  int ret;
  clockserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = DELAY;
  req.data = ticks;
  Send(tid, (char *)&req, sizeof(clockserver_request), (char *)&ret,
       sizeof(int));
  return ret;
}

int DelayUntil(int tid, int ticks) {
  int ret;
  clockserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = DELAYUNTIL;
  req.data = ticks;
  Send(tid, (char *)&req, sizeof(clockserver_request), (char *)&ret,
       sizeof(int));
  return ret;
}
