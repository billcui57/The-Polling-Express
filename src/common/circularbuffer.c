#include "circularbuffer.h"

void cb_init(circular_buffer *cb, void *buffer, size_t capacity, size_t size) {
  cb->buffer = buffer;
  memset(buffer, 0, sizeof(char) * capacity * size);
  cb->buffer_end = (char *)cb->buffer + capacity * size;
  cb->capacity = capacity;
  cb->count = 0;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;
  cb->size = size;
}

int cb_push_back(circular_buffer *cb, void *item, bool aggressive) {

  KASSERT(cb->buffer != NULL, "CB uninitialized");

  if (cb->count == cb->capacity) {
    if (aggressive) {
      char tmp[cb->size];
      cb_pop_front(cb, tmp);
    } else {
      return CB_FULL;
    }
  }

  memcpy(cb->head, item, cb->size);
  cb->head = (char *)cb->head + cb->size;
  if (cb->head == cb->buffer_end)
    cb->head = cb->buffer;
  cb->count++;
  return 0;
}

int cb_pop_front(circular_buffer *cb, void *item) {
  KASSERT(cb->buffer != NULL, "CB uninitialized");

  if (cb->count == 0) {
    return CB_EMPTY;
  }

  memcpy(item, cb->tail, cb->size);
  cb->tail = (char *)cb->tail + cb->size;
  if (cb->tail == cb->buffer_end)
    cb->tail = cb->buffer;
  cb->count--;
  return 0;
}

bool cb_is_empty(circular_buffer *cb) {
  KASSERT(cb->buffer != NULL, "CB uninitialized");
  return cb->count == 0;
}

bool cb_is_full(circular_buffer *cb) {
  KASSERT(cb->buffer != NULL, "CB uninitialized");
  return cb->count == cb->capacity;
}

int cb_to_array(circular_buffer *cb, void *arr) {
  KASSERT(cb->buffer != NULL, "CB uninitialized");

  int len = cb->count;

  void *cur = cb->tail;

  for (unsigned int i = 0; i < len; i++) {

    memcpy(arr, cur, cb->size);
    arr = (char *)arr + cb->size;
    cur = (char *)cur + cb->size;

    if (cur == cb->buffer_end) {
      cur = cb->buffer;
    }
  }
  return len;
}

void cb_clear(circular_buffer *cb) {
  KASSERT(cb->buffer != NULL, "CB uninitialized");
  cb_init(cb, cb->buffer, cb->capacity, cb->size);
}