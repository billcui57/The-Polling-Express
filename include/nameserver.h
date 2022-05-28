#pragma once

#include <hashtable.h>
#include <kprintf.h>
#include <my_assert.h>
#include <syscall.h>
#include <task.h>

#define MAX_NAMES 1000

#define MAX_BODY_LENGTH 128

#define REQUEST_REGISTER_AS 1
#define REQUEST_WHO_IS 2
typedef char request_type;

#define RESPONSE_GOOD 0
#define RESPONSE_NAME_DNE 1
typedef char response_type;

typedef struct nameserver_request {
  request_type type;
  char *body;
} nameserver_request;

typedef struct nameserver_response {
  response_type type;
  char *body;
} nameserver_response;

void request_init(nameserver_request *rq, request_type type, char *body);

void response_init(nameserver_response *rs, response_type type, char *body);

void nameserver();