#include "circularbuffer.h"
#include <assert.h>
#include <stdio.h>

void basic() {

  int MAX_CAPACITY = 3;
  int backing[MAX_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, (void *)backing, MAX_CAPACITY, sizeof(int));

  int a = 1;
  int b = 2;
  int c = 3;

  cb_push_back(&cb, (void *)&a, false);
  cb_push_back(&cb, (void *)&b, false);
  cb_push_back(&cb, (void *)&c, false);

  int res;
  cb_pop_front(&cb, (void *)&res);
  assert(res == a);
  cb_pop_front(&cb, (void *)&res);
  assert(res == b);
  cb_pop_front(&cb, (void *)&res);
  assert(res == c);
}

void has_struct() {

  typedef struct test_struct {
    int left;
    char right;
  } test_struct;

  int MAX_CAPACITY = 3;
  char backing[MAX_CAPACITY * sizeof(test_struct)];
  circular_buffer cb;
  cb_init(&cb, (void *)backing, MAX_CAPACITY, sizeof(test_struct));

  test_struct a;
  a.left = 2;
  a.right = 'a';
  test_struct b;
  a.left = 3;
  a.right = 'b';
  test_struct c;
  a.left = 4;
  a.right = 'c';

  cb_push_back(&cb, (void *)&a, false);
  cb_push_back(&cb, (void *)&b, false);
  cb_push_back(&cb, (void *)&c, false);

  test_struct res;
  cb_pop_front(&cb, (void *)&res);
  assert(res.left == a.left);
  assert(res.right == a.right);
  cb_pop_front(&cb, (void *)&res);
  assert(res.left == b.left);
  assert(res.right == b.right);
  cb_pop_front(&cb, (void *)&res);
  assert(res.left == c.left);
  assert(res.right == c.right);
}

int stress() {
  int MAX_CAPACITY = 3;
  int backing[MAX_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, (void *)backing, MAX_CAPACITY, sizeof(int));

  for (int i = 0; i < 10000; i++) {
    int a = 1;
    int b = 2;
    int c = 3;

    cb_push_back(&cb, (void *)&a, false);
    cb_push_back(&cb, (void *)&b, false);
    cb_push_back(&cb, (void *)&c, false);

    int res;
    cb_pop_front(&cb, (void *)&res);
    assert(res == a);
    cb_pop_front(&cb, (void *)&res);
    assert(res == b);
    cb_pop_front(&cb, (void *)&res);
    assert(res == c);
  }
}

int struct_stress() {
  typedef struct test_struct {
    int left;
    char right;
  } test_struct;

  int MAX_CAPACITY = 3;
  char backing[MAX_CAPACITY * sizeof(test_struct)];
  circular_buffer cb;
  cb_init(&cb, (void *)backing, MAX_CAPACITY, sizeof(test_struct));

  for (int i = 0; i < 1000; i++) {
    test_struct a;
    a.left = 2;
    a.right = 'a';
    test_struct b;
    a.left = 3;
    a.right = 'b';
    test_struct c;
    a.left = 4;
    a.right = 'c';

    cb_push_back(&cb, (void *)&a, false);
    cb_push_back(&cb, (void *)&b, false);
    cb_push_back(&cb, (void *)&c, false);

    test_struct res;
    cb_pop_front(&cb, (void *)&res);
    assert(res.left == a.left);
    assert(res.right == a.right);
    cb_pop_front(&cb, (void *)&res);
    assert(res.left == b.left);
    assert(res.right == b.right);
    cb_pop_front(&cb, (void *)&res);
    assert(res.left == c.left);
    assert(res.right == c.right);
  }
}

int main() {
  basic();
  has_struct();
  struct_stress();
  stress();
  return 0;
}