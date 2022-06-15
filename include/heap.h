#pragma once

// min heap
typedef struct {
  void **items;
  int len;
} heap;

void heap_init(heap *h, void **backing);
void heap_add(heap *h, void *item);
void *heap_peek(heap *h);
void heap_pop(heap *h);
