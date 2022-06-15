#include "circularbuffer.h"
#include <assert.h>
#include <stdio.h>

void basic() {

  int MAX_CAPACITY = 3;
  void *backing[MAX_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, backing, MAX_CAPACITY);

  int a = 1;
  int b = 2;
  int c = 3;

  cb_push_back(&cb, (void *)a, false);
  cb_push_back(&cb, (void *)b, false);
  cb_push_back(&cb, (void *)c, false);

  void *test;
  cb_pop_front(&cb, &test);
  assert((int)test == a);
  cb_pop_front(&cb, &test);
  assert((int)test == b);
  cb_pop_front(&cb, &test);
  assert((int)test == c);
}

void shift() {

  int MAX_CAPACITY = 3;
  void *backing[MAX_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, backing, MAX_CAPACITY);

  int a = 1;
  int b = 2;
  int c = 3;
  void *test;

  cb_push_back(&cb, (void *)a, false);
  cb_pop_front(&cb, &test);
  assert((int)test == a);
  cb_push_back(&cb, (void *)b, false);
  cb_pop_front(&cb, &test);
  assert((int)test == b);
  cb_push_back(&cb, (void *)c, false);
  cb_pop_front(&cb, &test);
  assert((int)test == c);
  cb_push_back(&cb, (void *)a, false);
  cb_pop_front(&cb, &test);
  assert((int)test == a);
  cb_push_back(&cb, (void *)b, false);
  cb_pop_front(&cb, &test);
  assert((int)test == b);
  cb_push_back(&cb, (void *)c, false);
  cb_pop_front(&cb, &test);
  assert((int)test == c);
  cb_push_back(&cb, (void *)a, false);
  cb_pop_front(&cb, &test);
  assert((int)test == a);
  cb_push_back(&cb, (void *)b, false);
  cb_pop_front(&cb, &test);
  assert((int)test == b);
  cb_push_back(&cb, (void *)c, false);
  cb_pop_front(&cb, &test);
  assert((int)test == c);
}

int main() {
  basic();
  shift();
  return 0;
}