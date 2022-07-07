#include <hashtable.h>

unsigned int hash_function(hashtable *ht, char *key) {
  unsigned long hash = 5381;
  int c;

  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash % ht->capacity;
}

void ht_init(hashtable *ht, size_t capacity, void **arr) {
  ht->capacity = capacity;
  ht->arr = arr;

  memset(ht->arr, 0, capacity * sizeof(void *));
}

int ht_insert(hashtable *ht, char *key, void *value) {

  unsigned int index = hash_function(ht, key);

  (ht->arr)[index] = value;

  return 0;
}

int ht_get(hashtable *ht, char *key, void **ret) {
  unsigned int index = hash_function(ht, key);

  if ((ht->arr)[index] == NULL) {
    return E_KEY_MISSING;
  }

  *ret = (void *)(ht->arr)[index];

  return 0;
}
