#pragma once

#include <memory.h>
#include <my_assert.h>
#include <stddef.h>

#define CB_FULL 1
#define CB_EMPTY 2

typedef struct circular_buffer {
  void **buffer;     // data buffer
  void **buffer_end; // end of data buffer
  size_t capacity;   // maximum number of items in the buffer
  size_t count;      // number of items in the buffer
  void **head;       // pointer to head
  void **tail;       // pointer to tail
} circular_buffer;

void cb_init(circular_buffer *cb, void **buffer, size_t capacity);
int cb_push_back(circular_buffer *cb, void *item, bool aggressive);
int cb_pop_front(circular_buffer *cb, void **item);
