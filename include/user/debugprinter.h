#pragma once

#include "circularbuffer.h"
#include "clockserver.h"
#include "kprintf.h"
#include "stdbool.h"
#include "string.h"
#include "syscall.h"

#define MAX_DEBUG_STRING_LEN 100
#define MAX_DEBUG_LINES 30

extern circular_buffer *debug_cb;
extern bool debug_changed;
extern int debug_index;

void debugprint(char *str);
void debugprinter();