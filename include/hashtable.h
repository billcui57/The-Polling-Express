#pragma once

#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
typedef struct hashtable {
  void **arr;
  size_t capacity;
} hashtable;

#define E_COLLISION 1
#define E_KEY_MISSING 2
#define NIN 0

void ht_init(hashtable *ht, size_t capacity, void **arr);

int ht_insert(hashtable *ht, char *key, void *value);

int ht_get(hashtable *ht, char *key, void **ret);
