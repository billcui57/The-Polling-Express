#include <kprintf.h>
#include <syscall.h>
#include <user.h>
#include <timer.h>

void task_k1init() {
  int ret;
  ret = Create(-10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(-10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  printf(&pc, "FirstUserTask: exiting\r\n");
  Exit();
}

void task_k1test() {
  int me = MyTid();
  int parent = MyParentTid();
  printf(&pc, "Me: %d Parent: %d \r\n", me, parent);
  Yield();
  printf(&pc, "Me: %d Parent: %d \r\n", me, parent);
  Exit();
}

void task_k2perf() {
  int sz[] = {4, 64, 256};
  int echo;
  int send;
  unsigned int time_diff;
  for (int i = 0 ; i<3; i ++){
    echo = Create(10, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff, sizeof(unsigned int));
    printf(&pc, "noopt cache R %d %d \r\n",sz[i], (time_diff * 10 )/ 508);
  }
  for (int i = 0 ; i<3; i ++){
    echo = Create(-5, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff, sizeof(unsigned int));
    printf(&pc, "noopt cache S %d %d \r\n",sz[i], (time_diff * 10 )/ 508);
  }
}

unsigned int read_timer3() {
  return *(unsigned int *)(TIMER3_BASE + VAL_OFFSET);
}

void task_echo() {
  char buf[256];
  int who;
  for(int i = 0; i<100; i++) {
    //printf(&pc, "Echo will call Receive\r\n");
    int len = Receive(&who, buf, 256);
    Reply(who, buf, len);
  }
}

void task_send(){
  int params[2];
  int who;
  Receive(&who, (char *)params, 2 * sizeof(int));
  int len = params[0];
  int echo = params[1];
  char buf[256];
  for (int i=0;i<len;i++){
    buf[i] = i & 0xF;
  }
  char rbuf[256];
  unsigned int start = read_timer3();
  for (int i=0;i<100;i++){
    //printf(&pc, "Send will call Send\r\n");
    Send(echo, buf, len, rbuf, 256);
  }
  unsigned int end = start - read_timer3();
  Reply(who, (char *)&end, sizeof(unsigned int));
}
