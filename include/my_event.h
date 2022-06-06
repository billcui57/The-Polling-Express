#pragma once

#include "task.h"

// put list of common events here
enum Event {
  ANY_EVENT,
  TIMER_TICK,
  NUMBER_OF_EVENTS // must always be last. First element must either
                   // be =0 or no
                   // =
};

// #define MAX_TASKS_PER_EVENT 50

// typedef struct event_queue{

//   TCB** backing;

// } event_queue;
