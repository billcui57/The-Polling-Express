#include "circularbuffer.h"

void cb_init(circular_buffer *cb, void **buffer, size_t capacity) {

  cb->buffer = buffer;
  memset(buffer, 0, capacity * sizeof(void *));
  cb->buffer_end = cb->buffer + capacity;
  cb->capacity = capacity;
  cb->count = 0;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;
}

int cb_push_back(circular_buffer *cb, void *item, bool aggressive) {

  if (cb->count == cb->capacity) {
    if (aggressive) {
      void *junk;
      cb_pop_front(cb, &junk);
    } else {
      return CB_FULL;
    }
  }

  *(cb->head) = item;
  cb->head++;
  if (cb->head == cb->buffer_end)
    cb->head = cb->buffer;
  cb->count++;
  return 0;
}

int cb_pop_front(circular_buffer *cb, void **item) {

  if (cb->count == 0) {
    return CB_EMPTY;
  }

  *item = *(cb->tail);

  cb->tail++;
  if (cb->tail == cb->buffer_end)
    cb->tail = cb->buffer;
  cb->count--;
  return 0;
}
