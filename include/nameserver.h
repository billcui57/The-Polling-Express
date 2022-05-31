#pragma once

#include <hashtable.h>
#include <my_assert.h>
#include <syscall.h>
#include <task.h>

#define MAX_NAMES 1000

#define MAX_BODY_LENGTH 128

#define REQUEST_REGISTER_AS 1
#define REQUEST_WHO_IS 2
typedef int nameserver_request_type;

#define RESPONSE_GOOD 0
#define RESPONSE_NAME_DNE -2
typedef int nameserver_response_type;

typedef struct nameserver_request {
  nameserver_request_type type;
  char body[MAX_BODY_LENGTH];
  unsigned int body_length;
} nameserver_request;

typedef struct nameserver_response {
  nameserver_response_type type;
  char body[MAX_BODY_LENGTH];
  unsigned int body_length;
} nameserver_response;

extern task_tid nameserver_tid;

void nameserver_request_init(nameserver_request *rq,
                             nameserver_request_type type, const char *body,
                             unsigned int body_length);

void nameserver_response_init(nameserver_response *rs,
                              nameserver_response_type type, const char *body,
                              unsigned int body_length);

int get_tid_from_nameserver_response(nameserver_response *rs);

void nameserver();
