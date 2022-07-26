#pragma once

#include "task.h"

// put list of common events here
enum Event {
  BREAK_IDLE,
  TIMER_TICK,
  UART1_RX_INTR,
  UART1_TX_INTR,
  UART2_TX_HALF_EMPTY,
  UART2_RX_INCOMING,
  NUMBER_OF_EVENTS // must always be last. First element
                   // must either be =0 or no
                   // =
};

// #define MAX_TASKS_PER_EVENT 50

// typedef struct event_queue{

//   TCB** backing;

// } event_queue;
