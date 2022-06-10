#pragma once

#include "circularbuffer.h"
#include "kprintf.h"
#include "my_assert.h"
#include "my_event.h"
#include "ts7200.h"

typedef enum {
  NOTIFIER_TX_GOOD,
  SEND_CHAR,
  RELEASE_LOCK
} uartserver_request_type;

typedef enum { GOOD } uartserver_response_type;

typedef enum { CAN_SEND, NO_SEND } uartserver_state;

typedef struct {
  uartserver_request_type type;
  int data;
} uartserver_request;

typedef struct {
  uartserver_response_type type;
  int data;
} uartserver_response;

void uart_com2_tx_server();