#include <user.h>

void task_k1init() {
  int ret;
  ret = Create(-10, nameserver);
  ret = Create(-10, rpsserver);

  Create(-10, task1);
  Create(-10, task2);
}

void task1() {
  task_tid rpsserver_tid;
  do {

    char query_string[] = "rpsserver";
    rpsserver_tid = WhoIs(query_string);
    printf(&pc, "stucks\r\n");
  } while (rpsserver_tid == -1);

  printf(&pc, "rps tid %d\r\n", rpsserver_tid);

  rpsserver_request rq;
  rpsserver_request_init(&rq, REQUEST_SIGNUP, "", 0);

  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(rpsserver_response)];

  Send(rpsserver_tid, request_buffer, sizeof(rpsserver_request),
       response_buffer, sizeof(rpsserver_response));

  nameserver_response *response =
      (nameserver_response *)response_buffer; // Deserialize

  printf(&pc, "1: I am in game %d\r\n", response->body[0]);
}

void task2() {
  task_tid rpsserver_tid;
  do {
    char query_string[] = "rpsserver";
    rpsserver_tid = WhoIs(query_string);
    printf(&pc, "stucks\r\n");
  } while (rpsserver_tid == -1);

  printf(&pc, "rps tid %d\r\n", rpsserver_tid);

  rpsserver_request rq;
  rpsserver_request_init(&rq, REQUEST_SIGNUP, "", 0);

  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(rpsserver_response)];

  Send(rpsserver_tid, request_buffer, sizeof(rpsserver_request),
       response_buffer, sizeof(rpsserver_response));

  nameserver_response *response =
      (nameserver_response *)response_buffer; // Deserialize

  printf(&pc, "2: I am in game %d\r\n", response->body[0]);
}
