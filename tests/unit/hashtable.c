#include "hashtable.h"
#include <assert.h>
#include <stdio.h>

void basic() {
  int status = 0;
  size_t capacity = 1000;
  void *backing[capacity];

  struct hashtable ht;

  ht_init(&ht, capacity, backing);

  // for (unsigned int i = 0; i < ht.capacity; i++) {
  //   for (unsigned int j = 0; j < ht.size; j++) {
  //     printf("%d|", *(char *)(ht.arr + (ht.size * i) + j));
  //   }
  // }

  // printf("===============================\r\n");

  unsigned int a = 1;
  status = ht_insert(&ht, "hello", (void *)a);
  assert(status != E_COLLISION);

  // for (unsigned int i = 0; i < ht.capacity; i++) {
  //   for (unsigned int j = 0; j < ht.size; j++) {
  //     printf("%d|", *(char *)(ht.arr + (ht.size * i) + j));
  //   }
  // }

  void *b;
  status = ht_get(&ht, "hello", &b);
  // printf("===============================\r\n");

  // for (unsigned int i = 0; i < ht.capacity; i++) {
  //     printf("%d|", ht.arr[i]);
  // }

  assert(status != E_KEY_MISSING);

  assert(a == (unsigned int)b);
}

// void override() {
//   int status = 0;
//   size_t capacity = 1000;
//   void *backing[capacity];

//   struct hashtable ht;

//   ht_init(&ht, capacity, &backing);

//   unsigned int a = 1;
//   status = ht_insert(&ht, "hello", (void *)a);
//   assert(status != E_COLLISION);
//   unsigned int b = 2;
//   status = ht_insert(&ht, "hello", (void *)b);
//   assert(status == E_COLLISION);
//   unsigned int c;
//   status = ht_get(&ht, "hello", (void *)&c);
//   assert(status != E_KEY_MISSING);

//   assert(b == c);
// }

int main() {
  basic();
  // override();
  return 0;
}
