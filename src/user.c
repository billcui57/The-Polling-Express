#include <kprintf.h>
#include <syscall.h>
#include <timer.h>
#include <user.h>

void task_k1init() {
  int ret;
  ret = Create(-10, nameserver);

  Create(-10, task1);
  // Create(-10, task2);
}

void task1() {
  while (RegisterAs("abc") != 0)
    ;

  for (;;) {

    task_tid tid = WhoIs("abc");

    if (tid == -1) {
      printf(&pc, "ERROR\r\n");
    } else {

      if (tid == 3) {
        while (RegisterAs("abc") != 0)
          ;
      }
      printf(&pc, "%d\r\n", tid);
    }
  }
}

void task2() {
  while (RegisterAs("abc") != 0)
    ;
  for (;;) {

    task_tid tid = WhoIs("abc");

    if (tid == -1) {
      printf(&pc, "ERROR\r\n");
    } else {

      if (tid == 2) {
        while (RegisterAs("abc") != 0)
          ;
      }

      printf(&pc, "%d\r\n", tid);
    }
  }
}

void task_k2perf() {
  int sz[] = {4, 64, 256};
  int echo;
  int send;
  unsigned int time_diff;
  for (int i = 0; i < 3; i++) {
    echo = Create(10, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    printf(&pc, "noopt cache R %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
  for (int i = 0; i < 3; i++) {
    echo = Create(-5, task_echo);
    int params[] = {sz[i], echo};
    send = Create(5, task_send);
    Send(send, (char *)&params, 2 * sizeof(int), (char *)&time_diff,
         sizeof(unsigned int));
    printf(&pc, "noopt cache S %d %d \r\n", sz[i], (time_diff * 10) / 508);
  }
}

unsigned int read_timer3() {
  return *(unsigned int *)(TIMER3_BASE + VAL_OFFSET);
}

void task_echo() {
  char buf[256];
  int who;
  for (int i = 0; i < 100; i++) {
    // printf(&pc, "Echo will call Receive\r\n");
    int len = Receive(&who, buf, 256);
    Reply(who, buf, len);
  }
}

void task_send() {
  int params[2];
  int who;
  Receive(&who, (char *)params, 2 * sizeof(int));
  int len = params[0];
  int echo = params[1];
  char buf[256];
  for (int i = 0; i < len; i++) {
    buf[i] = i & 0xF;
  }
  char rbuf[256];
  unsigned int start = read_timer3();
  for (int i = 0; i < 100; i++) {
    // printf(&pc, "Send will call Send\r\n");
    Send(echo, buf, len, rbuf, 256);
  }
  unsigned int end = start - read_timer3();
  Reply(who, (char *)&end, sizeof(unsigned int));

  nameserver_request rq;

  char body[4] = {'a', 'b', 'c', '\0'};
  request_init(&rq, REQUEST_REGISTER_AS, body, 4);

  char *request_buffer = (char *)&rq; // Serialize

  char receive_msg[MAX_MSG_LEN];
  Send(1, request_buffer, sizeof(nameserver_request), receive_msg, MAX_MSG_LEN);

  for (;;) {

    char body[4] = {'a', 'b', 'c', '\0'};
    request_init(&rq, REQUEST_WHO_IS, body, 4);
    char *request_buffer = (char *)&rq; // Serialize

    char response_buffer[sizeof(nameserver_response)];
    Send(1, request_buffer, sizeof(nameserver_request), response_buffer,
         sizeof(nameserver_response));

    nameserver_response *response =
        (nameserver_response *)response_buffer; // Deserialize

    if (response->type == RESPONSE_ERROR) {
      printf(&pc, "An error has occurred\r\n");
    } else {
      printf(&pc, "%d\r\n", (response->body)[0]);
    }
  }
}

void task2() {

  nameserver_request rq;

  char body[4] = {'b', 'c', 'd', '\0'};
  request_init(&rq, REQUEST_REGISTER_AS, body, 4);

  char *request_buffer = (char *)&rq; // Serialize

  char receive_msg[MAX_MSG_LEN];
  Send(1, request_buffer, sizeof(nameserver_request), receive_msg, MAX_MSG_LEN);

  for (;;) {

    // Serialize
    char body[4] = {'b', 'c', 'd', '\0'};
    request_init(&rq, REQUEST_WHO_IS, body, 4);
    char *request_buffer = (char *)&rq;

    char response_buffer[sizeof(nameserver_response)];
    Send(1, request_buffer, sizeof(nameserver_request), response_buffer,
         sizeof(nameserver_response));

    nameserver_response *response =
        (nameserver_response *)response_buffer; // Deserialize
    printf(&pc, "%d\r\n", (response->body)[0]);
  }
}