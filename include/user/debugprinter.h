#pragma once

#include "circularbuffer.h"
#include "clockserver.h"
#include "kprintf.h"
#include "stdbool.h"
#include "string.h"
#include "syscall.h"

#define MAX_DEBUG_STRING_LEN 100
#define MAX_DEBUG_LINES 30

#define PATH_WORKER_DEBUG 0b00000001
#define DISPATCH_SERVER_DEBUG 0b00000010
#define CRITICAL_DEBUG 0b00000100
#define NAVIGATION_SERVER_DEBUG 0b00001000
#define STRAIGHT_PATH_WORKER_DEBUG 0b00010000
#define TRAIN_SERVER_DEBUG 0b00100000
#define VERBOSE_DEBUG 0b01000000
#define RESERVATION_DEBUG 0b10000000

#define FILTER                                                                 \
  (CRITICAL_DEBUG | NAVIGATION_SERVER_DEBUG | STRAIGHT_PATH_WORKER_DEBUG)

extern circular_buffer *debug_cb;
extern bool debug_changed;
extern int debug_index;

void debugprint(char *str, int verboseness);