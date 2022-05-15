#pragma once

#include "hal.h"
#include "stdbool.h"
#include "stdint.h"

#define KASSERT(expr, str)                                                     \
  do {                                                                         \
    if (!(expr)) {                                                             \
      my_assert(str);                                                          \
    }                                                                          \
  } while (0)

void assert_init(uart *u);
void my_assert(char *str);
