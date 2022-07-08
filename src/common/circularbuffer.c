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

bool cb_is_empty(circular_buffer *cb) { return cb->count == 0; }

bool cb_is_full(circular_buffer *cb) { return cb->count == cb->capacity; }

int cb_shallow_linear_search(circular_buffer *cb, void *find) {

  void **cur = cb->tail;

  for (unsigned int i = 0; i < cb->count; i++) {

    if (*cur == find) {
      return cur - cb->buffer;
    }

    cur++;
    if (cur == cb->buffer_end) {
      cur = cb->buffer;
    }
  }

  return -1;
}

void cb_to_array(circular_buffer *cb, void **arr) {
  void **cur = cb->tail;

  for (unsigned int i = 0; i < cb->count; i++) {

    arr[i] = *cur;

    cur++;
    if (cur == cb->buffer_end) {
      cur = cb->buffer;
    }
  }
}
