#pragma once

#include "circularbuffer.h"
#include "clockserver.h"
#include "kprintf.h"
#include "stdbool.h"
#include "string.h"
#include "syscall.h"

#define MAX_DEBUG_STRING_LEN 100

void debugprint(char *str);
void debugprinter();