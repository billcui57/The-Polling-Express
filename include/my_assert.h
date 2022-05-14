#include "stdint.h"
#include "stdbool.h"
#include "hal.h"

#define KASSERT(expr, id) \
  do                      \
  {                       \
    if (!(expr))          \
    {                     \
      my_assert(id);      \
    }                     \
  } while (0)

void assert_init(uart *u);

